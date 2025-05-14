#include "pathname.h"
#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include "unixfilesystem.h"
#include <string.h>
#include <stdlib.h>

#define PATH_MAX_LEN 1024

/** Busca el número de inodo correspondiente a un path absoluto.
  Params: fs (struct unixfilesystem*), pathname (char*).
  Returns: inumber si éxito, -1 si error (path inválido o no encontrado). */
int pathname_lookup(struct unixfilesystem *fs, const char *pathname) {
    if (!fs || !pathname || pathname[0] != '/') return -1;

    if (strcmp(pathname, "/") == 0) return ROOT_INUMBER;

    char buffer[PATH_MAX_LEN];
    strncpy(buffer, pathname, PATH_MAX_LEN);
    buffer[PATH_MAX_LEN - 1] = '\0';

    int current_inumber = ROOT_INUMBER;
    char *next = strtok(buffer + 1, "/");

    while (next) {
        struct direntv6 entry;
        if (strlen(next) >= sizeof(entry.d_name)) return -1;

        if (directory_findname(fs, next, current_inumber, &entry) < 0) return -1;

        current_inumber = entry.d_inumber;
        next = strtok(NULL, "/");
    }

    return current_inumber;
}