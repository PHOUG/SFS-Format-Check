#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "colors.h"
#include "structures.h"

// ==== Function Prototypes ====

void write_superblock(FILE *disk);
void write_bitmap(FILE *disk);
void write_inodes(FILE *disk);

// ==== Main format.c ====

int format_disk() {
    char disk_name[256];
    printf("\n["BOLD"ENTER"RESET"] "BOLD"Введите название"RESET": ");
    scanf("%s", disk_name);
    printf("["BOLD"PROCESS"RESET"] "BOLD"Форматирование"RESET": создание файла %s...\n", disk_name);
    FILE *disk = fopen(disk_name, "wb");
    if (!disk) {
        perror("fopen");
        return 1;
    }

    // Заполнить файл нулями (1MB)
    fseek(disk, DISK_SIZE - 1, SEEK_SET);
    fputc('\0', disk);

    // Вернуться к началу
    fseek(disk, 0, SEEK_SET);

    printf("["BOLD"PROCESS"RESET"] "BOLD"Форматирование"RESET": создание суперблока, битовой карты и inode-таблицы...\n");

    
    printf("["BOLD"PROCESS"RESET"] "BOLD"Форматирование"RESET": создание суперблока...\n");
    write_superblock(disk);
    printf("["BOLD"PROCESS"RESET"] "BOLD"Форматирование"RESET": создание битовой карты...\n");
    write_bitmap(disk);
    printf("["BOLD"PROCESS"RESET"] "BOLD"Форматирование"RESET": создание inode-таблицы...\n");
    write_inodes(disk);

    fclose(disk);
    printf("["GREEN""BOLD"OK"RESET"] "BOLD"Форматирование завершено"RESET": %s создан.\n\n", disk_name);
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
