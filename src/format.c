#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "colors.h"

#define DISK_SIZE      (1024 * 1024)   // 1 MB
#define BLOCK_SIZE     512
#define TOTAL_BLOCKS   (DISK_SIZE / BLOCK_SIZE)

#define SUPERBLOCK_BLOCK     0
#define BITMAP_BLOCK_START   1
#define BITMAP_BLOCK_COUNT   2
#define INODE_BLOCK_START    3
#define INODE_BLOCK_COUNT    16
#define DATA_BLOCK_START     (SUPERBLOCK_BLOCK + BITMAP_BLOCK_COUNT + INODE_BLOCK_COUNT)

#define MAX_FILENAME_LENGTH  28
#define MAX_INODES           ((INODE_BLOCK_COUNT * BLOCK_SIZE) / sizeof(Inode))
#define MAX_BLOCKS_PER_FILE  8

// ==== Structures ====

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

// ==== Function Prototypes ====

void write_superblock(FILE *disk);
void write_bitmap(FILE *disk);
void write_inodes(FILE *disk);

// ==== Main format.c ====

int main() {
    FILE *disk = fopen("disk.img", "wb");
    if (!disk) {
        perror("fopen");
        return 1;
    }

    // Заполнить файл нулями (1MB)
    fseek(disk, DISK_SIZE - 1, SEEK_SET);
    fputc('\0', disk);

    // Вернуться к началу
    fseek(disk, 0, SEEK_SET);

    write_superblock(disk);
    write_bitmap(disk);
    write_inodes(disk);

    fclose(disk);
    printf("["GREEN""BOLD"OK"RESET"] "BOLD"Форматирование завершено"RESET": disk.img создан.\n");
    return 0;
}

// ==== Function Realization ====

void write_superblock(FILE *disk) {
    SuperBlock sb = {
        .magic = 0x20240507,
        .block_size = BLOCK_SIZE,
        .total_blocks = TOTAL_BLOCKS,
        .inode_table_start = INODE_BLOCK_START,
        .data_block_start = DATA_BLOCK_START,
    };
    fseek(disk, SUPERBLOCK_BLOCK * BLOCK_SIZE, SEEK_SET);
    fwrite(&sb, sizeof(SuperBlock), 1, disk);
}

void write_bitmap(FILE *disk) {
    uint8_t bitmap[BLOCK_SIZE * BITMAP_BLOCK_COUNT] = {0};

    // Пометить служебные блоки как занятые
    for (int i = 0; i < DATA_BLOCK_START; ++i) {
        bitmap[i / 8] |= (1 << (i % 8));
    }

    fseek(disk, BITMAP_BLOCK_START * BLOCK_SIZE, SEEK_SET);
    fwrite(bitmap, sizeof(bitmap), 1, disk);
}

void write_inodes(FILE *disk) {
    Inode inode = {0}; // все inodes пустые
    fseek(disk, INODE_BLOCK_START * BLOCK_SIZE, SEEK_SET);
    for (unsigned int i = 0; i < MAX_INODES; ++i) {
        fwrite(&inode, sizeof(Inode), 1, disk);
    }
}
