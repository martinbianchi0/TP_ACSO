#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include "file.h"
#include "unixfilesystem.h"
#include <string.h>

#define ENTRY_SIZE sizeof(struct direntv6)
#define BLOCK_SIZE DISKIMG_SECTOR_SIZE
#define MAX_NAME_LEN 14

/** Busca una entrada de nombre `name` en el directorio `dirinumber`.
  Params: fs (unixfilesystem*), name (char*), dirinumber (int), dirOut (struct direntv6*).
  Returns: 0 si encontró, -1 si error o no está. */
int directory_findname(struct unixfilesystem *fs, const char *name, int dirinumber, struct direntv6 *dirOut) {
    if (!fs || !name || !dirOut || dirinumber < 1) return -1;

    struct inode dir_inode;
    if (inode_iget(fs, dirinumber, &dir_inode) < 0) return -1;

    if (!(dir_inode.i_mode & IALLOC) || (dir_inode.i_mode & IFMT) != IFDIR) return -1;

    int size = inode_getsize(&dir_inode);
    if (size <= 0 || size % ENTRY_SIZE != 0) return -1;

    int total_blocks = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    unsigned char buffer[BLOCK_SIZE];

    for (int b = 0; b < total_blocks; b++) {
        int read_bytes = file_getblock(fs, dirinumber, b, buffer);
        if (read_bytes < 0) return -1;
        if (read_bytes == 0) continue;

        int entries = read_bytes / ENTRY_SIZE;
        struct direntv6 *entry = (struct direntv6 *)buffer;

        for (int i = 0; i < entries; i++) {
            if (entry[i].d_inumber == 0) continue;

            if (strncmp(name, entry[i].d_name, MAX_NAME_LEN) == 0) {
                *dirOut = entry[i];
                return 0;
            }
        }
    }

    return -1;
}