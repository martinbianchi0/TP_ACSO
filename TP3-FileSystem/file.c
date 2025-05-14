#include "file.h"
#include "inode.h"
#include "diskimg.h"
#include <string.h>

#define BLOCK_BYTES DISKIMG_SECTOR_SIZE

/** Lee el bloque `blockNum` lógico del archivo `inumber` y lo guarda en `buf`.
  Params: fs (unixfilesystem*), inumber (int), blockNum (int), buf (void*).
  Returns: cantidad válida de bytes leídos o -1 si error. */
int file_getblock(struct unixfilesystem *fs, int inumber, int blockNum, void *buf) {
    if (!fs || !buf || inumber < 1 || blockNum < 0) return -1;

    struct inode node;
    if (inode_iget(fs, inumber, &node) < 0) return -1;
    if (!(node.i_mode & IALLOC)) return -1;

    int file_size = inode_getsize(&node);
    if (file_size <= 0) return 0;

    int total_blocks = (file_size + BLOCK_BYTES - 1) / BLOCK_BYTES;
    if (blockNum >= total_blocks) return -1;

    int disk_block = inode_indexlookup(fs, &node, blockNum);
    if (disk_block < 0) return -1;

    if (disk_block == 0) {
        memset(buf, 0, BLOCK_BYTES);
    } else {
        if (diskimg_readsector(fs->dfd, disk_block, buf) != BLOCK_BYTES) return -1;
    }

    int remaining = file_size - blockNum * BLOCK_BYTES;
    return (remaining >= BLOCK_BYTES) ? BLOCK_BYTES : remaining;
}