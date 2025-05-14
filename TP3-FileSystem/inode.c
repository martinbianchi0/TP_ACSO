#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "inode.h"
#include "diskimg.h"
#include "unixfilesystem.h"

#define INODES_PER_SECTOR (DISKIMG_SECTOR_SIZE / sizeof(struct inode))
#define PTRS_PER_SECTOR   (DISKIMG_SECTOR_SIZE / sizeof(uint16_t))

/** Lee el inode identificado por `inumber` desde el sistema de archivos `fs`.
  Params: fs (struct unixfilesystem*), inumber (int), inp (struct inode*).
  Returns: 0 si éxito, -1 si error (número inválido o fallo de lectura). */
int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
    if (!fs || !inp || inumber < 1) return -1;

    int total_inodes = fs->superblock.s_isize * INODES_PER_SECTOR;
    if (inumber > total_inodes) return -1;

    int sector = INODE_START_SECTOR + (inumber - 1) / INODES_PER_SECTOR;
    int offset  = (inumber - 1) % INODES_PER_SECTOR;

    char buffer[DISKIMG_SECTOR_SIZE];
    if (diskimg_readsector(fs->dfd, sector, buffer) != DISKIMG_SECTOR_SIZE)
        return -1;

    struct inode *table = (struct inode *) buffer;
    *inp = table[offset];
    return 0;
}

/** Dado un bloque lógico de archivo, devuelve el número de sector físico asociado.
  Params: fs (struct unixfilesystem*), inp (struct inode*), fileBlockNum (int).
  Returns: número de bloque físico o -1 si error o fuera de rango. */
int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp, int fileBlockNum) {
    if (!fs || !inp || fileBlockNum < 0) return -1;

    // Archivo pequeño: hasta 8 bloques directos
    if (!(inp->i_mode & ILARG)) {
        return (fileBlockNum < 8) ? inp->i_addr[fileBlockNum] : -1;
    }

    int blocks_in_indirect = 7 * PTRS_PER_SECTOR;

    // Bloques en indirecciones simples (7 punteros a bloques de 256)
    if (fileBlockNum < blocks_in_indirect) {
        int indir_index = fileBlockNum / PTRS_PER_SECTOR;
        int offset = fileBlockNum % PTRS_PER_SECTOR;

        uint16_t sector = inp->i_addr[indir_index];
        if (sector == 0) return -1;

        uint16_t indirect_block[PTRS_PER_SECTOR];
        if (diskimg_readsector(fs->dfd, sector, indirect_block) != DISKIMG_SECTOR_SIZE)
            return -1;

        return indirect_block[offset];
    }

    // Bloques en doble indirección
    int relative = fileBlockNum - blocks_in_indirect;
    if (relative >= PTRS_PER_SECTOR * PTRS_PER_SECTOR) return -1;

    int first = relative / PTRS_PER_SECTOR;
    int second = relative % PTRS_PER_SECTOR;

    uint16_t double_sector = inp->i_addr[7];
    if (double_sector == 0) return -1;

    uint16_t double_block[PTRS_PER_SECTOR];
    if (diskimg_readsector(fs->dfd, double_sector, double_block) != DISKIMG_SECTOR_SIZE)
        return -1;

    uint16_t indirect_sector = double_block[first];
    if (indirect_sector == 0) return -1;

    uint16_t indirect_block[PTRS_PER_SECTOR];
    if (diskimg_readsector(fs->dfd, indirect_sector, indirect_block) != DISKIMG_SECTOR_SIZE)
        return -1;

    return indirect_block[second];
}

/** Devuelve el tamaño del archivo apuntado por `inp` en bytes.
  Params: inp (struct inode*).
  Returns: tamaño del archivo. */
int inode_getsize(struct inode *inp) {
    if (!inp) return -1;
    return (inp->i_size0 << 16) | inp->i_size1;
}