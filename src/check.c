#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "colors.h"
#include "structures.h"

// ==== Function Prototypes ====

void read_superblock(FILE *disk, SuperBlock *sb);
void read_bitmap(FILE *disk, uint8_t *bitmap);
void check_bitmap(const uint8_t *bitmap);
void check_superblock(const SuperBlock *sb);

// ==== Main check.c ====

int main() {
    FILE *disk = fopen("disk.img", "rb");
    if (!disk) {
        perror("fopen");
        return 1;
    }

    SuperBlock sb;
    read_superblock(disk, &sb);
    check_superblock(&sb);

    uint8_t bitmap[BLOCK_SIZE * BITMAP_BLOCK_COUNT];
    read_bitmap(disk, bitmap);
    check_bitmap(bitmap);

    fclose(disk);
    printf("["GREEN""BOLD"OK"RESET"] Проверка завершена.\n");
    return 0;
}

// ==== Function Realization ====

void read_superblock(FILE *disk, SuperBlock *sb) {
    fseek(disk, SUPERBLOCK_BLOCK * BLOCK_SIZE, SEEK_SET);
    fread(sb, sizeof(SuperBlock), 1, disk);
}

void read_bitmap(FILE *disk, uint8_t *bitmap) {
    fseek(disk, BITMAP_BLOCK_START * BLOCK_SIZE, SEEK_SET);
    fread(bitmap, BLOCK_SIZE * BITMAP_BLOCK_COUNT, 1, disk);
}

void check_bitmap(const uint8_t *bitmap) {
    for (int i = 0; i < DATA_BLOCK_START; ++i) {
        if (!(bitmap[i / 8] & (1 << (i % 8)))) {
            printf("["RED""BOLD"ERROR"RESET"] Служебный блок #%d не помечен как занятый\n", i);
        }
    }
    printf("["YELLOW""BOLD"INFO"RESET"] "BOLD"Битовая карта"RESET": проверка завершена\n");
}

void check_superblock(const SuperBlock *sb) {
    if (sb->magic != 0x20240507) {
        printf("["RED""BOLD"ERROR"RESET"] Неверная сигнатура суперблока (magic = 0x%x)\n", sb->magic);
        return;
    }
    if (sb->block_size != BLOCK_SIZE || sb->total_blocks != TOTAL_BLOCKS) {
        printf("["RED""BOLD"ERROR"RESET"] Параметры суперблока некорректны\n");
        return;
    }
    printf("["YELLOW""BOLD"INFO"RESET"] "BOLD"Суперблок"RESET": проверка пройдена\n");
}