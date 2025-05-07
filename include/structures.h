#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <stdint.h>
#include "define.h"

// ==== Disk Structures ====

typedef struct {
    uint32_t magic;
    uint32_t block_size;
    uint32_t total_blocks;
    uint32_t inode_table_start;
    uint32_t data_block_start;
} SuperBlock;

typedef struct {
    uint8_t used;
    char name[MAX_FILENAME_LENGTH];
    uint32_t size;
    uint32_t direct_blocks[MAX_BLOCKS_PER_FILE];
} Inode;

#endif // STRUCTURES_H
