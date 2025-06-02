#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/*
 * Este programa crea N procesos que forman un anillo.
 * Un valor entero se pasa de proceso en proceso, incrementándose en cada paso,
 * hasta volver al proceso que lo inició, donde se imprime el valor final.
 *
 * Uso: ./ring <num_procesos> <valor_inicial> <proceso_inicio>
 */
int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <num_procesos> <valor_inicial> <proceso_inicio>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const int num_procesos = atoi(argv[1]);
    const int valor_inicial = atoi(argv[2]);
    const int inicio = atoi(argv[3]);

    if (num_procesos < 1 || inicio < 1 || inicio > num_procesos) {
        fprintf(stderr, "Error en parámetros: <num_procesos> debe ser ≥ 1 y <proceso_inicio> debe estar entre 1 y la cantidad de procesos");
        exit(EXIT_FAILURE);
    }

    printf("Se crearán %d procesos, se enviará el valor %d desde proceso %d\n", num_procesos, valor_inicial, inicio);

    if (num_procesos == 1) {
        // Caso especial: n=1 (sin comunicación entre procesos)
        int resultado = valor_inicial + 1;
        printf("Valor final recibido: %d\n", resultado); // Formato esperado por el tester
        return 0;
    }

    // Creación de pipes
    int pipes[num_procesos][2];
    for (int i = 0; i < num_procesos; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("Error al crear pipe");
            exit(EXIT_FAILURE);
        }
    }

    // Creación de procesos hijos
    for (int i = 0; i < num_procesos; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("Error al crear proceso hijo");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) { // Código ejecutado por procesos hijos
            for (int j = 0; j < num_procesos; j++) {
                if (j != i) close(pipes[j][0]); // Solo lee del proceso anterior
                if (j != (i + 1) % num_procesos) close(pipes[j][1]); // Solo escribe en el siguiente proceso
            }

            int valor;
            if (i == inicio - 1) {
                valor = valor_inicial; // El proceso inicial recibe el valor inicial
            } else {
                read(pipes[i][0], &valor, sizeof(int)); // Leer del proceso anterior
            }

            valor++;
            write(pipes[(i + 1) % num_procesos][1], &valor, sizeof(int)); // Escribir en el siguiente proceso

            if (i == inicio - 1) {
                read(pipes[i][0], &valor, sizeof(int)); // Leer el valor final
                printf("Valor final recibido: %d\n", valor); // Formato esperado por el tester
            }

            close(pipes[i][0]);
            close(pipes[(i + 1) % num_procesos][1]);
            exit(EXIT_SUCCESS);
        }
    }

    // Código ejecutado por el proceso padre
    for (int i = 0; i < num_procesos; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for (int i = 0; i < num_procesos; i++) {
        wait(NULL);
    }

    return 0;
}