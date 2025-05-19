#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "colors.h"
#include "structures.h"

// ==== Function Prototypes ====

void read_superblock(FILE *disk, SuperBlock *sb);
void read_bitmap(FILE *disk, uint8_t *bitmap);
void read_inodes(FILE *disk, Inode *inodes);
void check_superblock(const SuperBlock *sb);
void check_bitmap(const uint8_t *bitmap);
int is_block_used(const uint8_t *bitmap, int block_index);
void check_inodes(const Inode *inodes, const uint8_t *bitmap);

// ==== Main check.c ====

int check_main() {
    FILE *disk = fopen("disk.img", "rb");
    if (!disk) {
        perror("fopen");
        return 1;
    }

    SuperBlock sb;
    uint8_t bitmap[BLOCK_SIZE * BITMAP_BLOCK_COUNT];
    Inode inodes[MAX_INODES];

    read_superblock(disk, &sb);
    check_superblock(&sb);

    read_bitmap(disk, bitmap);
    check_bitmap(bitmap);

    read_inodes(disk, inodes);
    check_inodes(inodes, bitmap);

    fclose(disk);
    printf("["GREEN""BOLD"OK"RESET"] Проверка завершена.\n\n");
    return 0;
}

void read_superblock(FILE *disk, SuperBlock *sb) {
    fseek(disk, SUPERBLOCK_BLOCK * BLOCK_SIZE, SEEK_SET);
    fread(sb, sizeof(SuperBlock), 1, disk);
}

void read_bitmap(FILE *disk, uint8_t *bitmap) {
    fseek(disk, BITMAP_BLOCK_START * BLOCK_SIZE, SEEK_SET);
    fread(bitmap, BLOCK_SIZE * BITMAP_BLOCK_COUNT, 1, disk);
}

void read_inodes(FILE *disk, Inode *inodes) {
    fseek(disk, INODE_BLOCK_START * BLOCK_SIZE, SEEK_SET);
    fread(inodes, sizeof(Inode), MAX_INODES, disk);
}

void check_superblock(const SuperBlock *sb) {
    printf("\n["BOLD"PROCESS"RESET"] Проверка суперблока...\n");
    if (sb->magic != 0x20240507) {
        printf("["RED""BOLD"ERROR"RESET"] Неверная сигнатура суперблока (magic = 0x%x)\n", sb->magic);
        return;
    }
    if (sb->block_size != BLOCK_SIZE || sb->total_blocks != TOTAL_BLOCKS) {
        printf("["RED""BOLD"ERROR"RESET"] Параметры суперблока некорректны\n");
        return;
    }
    printf("["YELLOW""BOLD"INFO"RESET"] "BOLD"Суперблок"RESET": проверка пройдена\n\n");
}

void check_bitmap(const uint8_t *bitmap) {
    printf("["BOLD"PROCESS"RESET"] Проверка битовой карты...\n");
    for (int i = 0; i < DATA_BLOCK_START; ++i) {
        if (!(bitmap[i / 8] & (1 << (i % 8)))) {
            printf("["RED""BOLD"ERROR"RESET"] Служебный блок #%d не помечен как занятый\n", i);
        }
    }
    printf("["YELLOW""BOLD"INFO"RESET"] "BOLD"Битовая карта"RESET": проверка пройдена\n\n");
}

int is_block_used(const uint8_t *bitmap, int block_index) {
    return bitmap[block_index / 8] & (1 << (block_index % 8));
}

void check_inodes(const Inode *inodes, const uint8_t *bitmap) {
    int used_blocks[TOTAL_BLOCKS] = {0};

    printf("["BOLD"PROCESS"RESET"] Проверка inode-таблицы...\n");
    for (unsigned int i = 0; i < MAX_INODES; ++i) {
        const Inode *inode = &inodes[i];

        if (!inode->used) continue;

        // Проверка имени
        if (strlen(inode->name) == 0 || strlen(inode->name) >= MAX_FILENAME_LENGTH) {
            printf("["RED""BOLD"ERROR"RESET"] Inode #%d: некорректное имя файла '%s'\n", i, inode->name);
        }

        // Проверка размера
        if (inode->size > MAX_BLOCKS_PER_FILE * BLOCK_SIZE) {
            printf("["RED""BOLD"ERROR"RESET"] Inode #%d: размер файла превышает допустимый: %u байт\n", i, inode->size);
        }

        // Проверка блоков
        for (int j = 0; j < MAX_BLOCKS_PER_FILE; ++j) {
            uint32_t block = inode->direct_blocks[j];
            if (block == 0) continue;

            if (block < DATA_BLOCK_START || block >= TOTAL_BLOCKS) {
                printf("["RED""BOLD"ERROR"RESET"] Inode #%d: недопустимый номер блока: %u\n", i, block);
                continue;
            }

            if (!is_block_used(bitmap, block)) {
                printf("["RED""BOLD"ERROR"RESET"] Inode #%d: блок #%u не помечен как занятый в bitmap\n", i, block);
            }

            if (used_blocks[block]) {
                printf("["RED""BOLD"ERROR"RESET"] Inode #%d: блок #%u уже используется другим inode\n", i, block);
            } else {
                used_blocks[block] = 1;
            }
        }
    }

    printf("["YELLOW""BOLD"INFO"RESET"] Inode-таблица: проверка завершена\n\n");
}
