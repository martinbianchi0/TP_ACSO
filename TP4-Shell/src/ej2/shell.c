#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_ARGS 64
#define MAX_CMDS 256

typedef struct {
    char *argv[MAX_ARGS + 1];
} Command;

void init_commands(Command *cmds) {
    for (int i = 0; i < MAX_CMDS; i++) {
        for (int j = 0; j <= MAX_ARGS; j++) {
            cmds[i].argv[j] = NULL;
        }
    }
}

void free_commands(Command *cmds, int num_cmds) {
    for (int i = 0; i < num_cmds; i++) {
        for (int j = 0; cmds[i].argv[j]; j++) {
            free(cmds[i].argv[j]);
            cmds[i].argv[j] = NULL;
        }
    }
}

void free_all(Command *cmds, int cmd_idx, int argc) {
    for (int j = 0; j < argc; j++) {
        free(cmds[cmd_idx].argv[j]);
        cmds[cmd_idx].argv[j] = NULL;
    }
    free_commands(cmds, cmd_idx);
}

int is_space(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

int parse_line(char *line, Command *cmds, int *num_cmds) {
    int argc = 0, cmd_idx = 0;
    char token[1024];
    int i = 0;
    char quote_char = '\0';
    *num_cmds = 0;

    while (*line) {
        while (is_space(*line)) line++;
        if (*line == '\0') break;

        if (*line == '|') {
            if (argc == 0) {
                write(STDERR_FILENO, "Error: sintaxis de pipes inválida\n", 35);
                free_all(cmds, cmd_idx, argc);
                return -1;
            }
            cmds[cmd_idx].argv[argc] = NULL;
            cmd_idx++;
            argc = 0;
            line++;
            continue;
        }

        i = 0;
        if (*line == '"' || *line == '\'') {
            quote_char = *line++;
            while (*line && *line != quote_char) {
                token[i++] = *line++;
            }
            if (*line != quote_char) {
                write(STDERR_FILENO, "Error: comillas sin cerrar o demasiados argumentos\n", 52);
                free_all(cmds, cmd_idx, argc);
                return -1;
            }
            line++; // cerrar comillas
        } else {
            while (*line && !is_space(*line) && *line != '|') {
                token[i++] = *line++;
            }
        }

        token[i] = '\0';

        if (argc >= MAX_ARGS) {
            write(STDERR_FILENO, "Error: demasiados argumentos\n", 29);
            free_all(cmds, cmd_idx, argc);
            return -1;
        }

        cmds[cmd_idx].argv[argc++] = strdup(token);
        if (!cmds[cmd_idx].argv[argc - 1]) {
            write(STDERR_FILENO, "Error: no se pudo asignar memoria\n", 34);
            free_all(cmds, cmd_idx, argc);
            return -1;
        }
    }

    if (argc == 0 && cmd_idx > 0) {
        write(STDERR_FILENO, "Error: sintaxis de pipes inválida\n", 35);
        free_all(cmds, cmd_idx, argc);
        return -1;
    }

    if (argc > 0) {
        cmds[cmd_idx].argv[argc] = NULL;
        cmd_idx++;
    }

    *num_cmds = cmd_idx;
    return 0;
}

int execute_commands(Command *cmds, int num_cmds) {
    if (num_cmds == 0) return 0;

    int pipes[2 * (num_cmds - 1)];
    pid_t pids[num_cmds];

    for (int i = 0; i < num_cmds - 1; i++) {
        if (pipe(pipes + i * 2) < 0) {
            write(STDERR_FILENO, "Error: pipe\n", 12);
            return -1;
        }
    }

    for (int i = 0; i < num_cmds; i++) {
        pids[i] = fork();
        if (pids[i] < 0) {
            write(STDERR_FILENO, "Error: fork\n", 12);
            return -1;
        } else if (pids[i] == 0) {
            if (i > 0) {
                dup2(pipes[(i - 1) * 2], STDIN_FILENO);
            }
            if (i < num_cmds - 1) {
                dup2(pipes[i * 2 + 1], STDOUT_FILENO);
            }

            for (int j = 0; j < 2 * (num_cmds - 1); j++) {
                close(pipes[j]);
            }

            execvp(cmds[i].argv[0], cmds[i].argv);
            write(STDERR_FILENO, "execvp error\n", 13);
            _exit(1);
        }
    }

    for (int i = 0; i < 2 * (num_cmds - 1); i++) {
        close(pipes[i]);
    }

    int status;
    for (int i = 0; i < num_cmds; i++) {
        waitpid(pids[i], &status, 0);
    }

    return 0;
}

int main() {
    char *line = NULL;
    size_t len = 0;
    Command cmds[MAX_CMDS];
    int num_cmds;

    init_commands(cmds);

    while (1) {
        //if (isatty(STDIN_FILENO)) {
        printf("Shell> ");
        fflush(stdout);
        //}

        ssize_t n = getline(&line, &len, stdin);
        if (n == -1) break;

        line[strcspn(line, "\n")] = '\0';

        if (strcmp(line, "exit") == 0) break;

        if (parse_line(line, cmds, &num_cmds) == 0) {
            execute_commands(cmds, num_cmds);
            free_commands(cmds, num_cmds);
        }
    }

    free(line);
    return 0;
}