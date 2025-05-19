#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "colors.h"
#include "structures.h"
#include "error.h"

// ==== Function Prototypes ====

void print_superblock(FILE *disk);
void print_bitmap(FILE *disk);
void print_inodes(FILE *disk);

// ==== Main dump.c ====

int dump_file()
{
    system("clear");
    if (error_flag)
    {
        printf(ERROR "" BOLD " Ошибка" RESET ": Диск повреждён\n");
    }
    char disk_name[256];
    printf(ENTER "" BOLD " Введите имя файла-диска" RESET ": ");
    scanf("%255s", disk_name);

    FILE *disk = fopen(disk_name, "rb");
    if (!disk)
    {
        printf(ERROR "" BOLD " Ошибка" RESET ": не удалось открыть файл %s\n", disk_name);
        return 1;
    }

    printf("\n" INFO "" BOLD " Считывание содержимого диска..." RESET "\n");

    print_superblock(disk);
    print_bitmap(disk);
    print_inodes(disk);

    fclose(disk);
    return 0;
}

void print_superblock(FILE *disk)
{
    SuperBlock sb;
    fseek(disk, SUPERBLOCK_BLOCK * BLOCK_SIZE, SEEK_SET);
    fread(&sb, sizeof(SuperBlock), 1, disk);

    printf("\n" INFO "" BOLD " SuperBlock " RESET "\n");
    printf(" magic            : 0x%X\n", sb.magic);
    printf(" block_size       : %u bytes\n", sb.block_size);
    printf(" total_blocks     : %u\n", sb.total_blocks);
    printf(" inode_table_start: %u\n", sb.inode_table_start);
    printf(" data_block_start : %u\n", sb.data_block_start);
}

void print_bitmap(FILE *disk)
{
    size_t bitmap_size = BLOCK_SIZE * BITMAP_BLOCK_COUNT;
    uint8_t *bitmap = malloc(bitmap_size);
    if (!bitmap)
    {
        printf(ERROR "" BOLD " Ошибка" RESET ": не удалось выделить память для битовой карты\n");
        return;
    }

    fseek(disk, BITMAP_BLOCK_START * BLOCK_SIZE, SEEK_SET);
    fread(bitmap, 1, bitmap_size, disk);

    printf("\n" INFO "" BOLD " Bitmap " RESET "\n");
    printf(" Блоки (0 – свободен, 1 – занят):\n");

    for (unsigned int i = 0; i < TOTAL_BLOCKS; ++i)
    {
        int bit = (bitmap[i / 8] >> (i % 8)) & 1;
        putchar(bit ? '1' : '0');
        if ((i + 1) % 64 == 0)
            putchar('\n');
    }
    putchar('\n');

    free(bitmap);
}

void print_inodes(FILE *disk)
{
    Inode inode;
    fseek(disk, INODE_BLOCK_START * BLOCK_SIZE, SEEK_SET);

    printf("\n" INFO "" BOLD " Inode Table " RESET "\n");

    int any_printed = 0;
    for (size_t i = 0; i < MAX_INODES; ++i)
    {
        fread(&inode, sizeof(Inode), 1, disk);
        if (!inode.used)
            continue;

        any_printed = 1;
        printf("\n" INFO "" BOLD " Inode #%zu " RESET "\n", i);
        printf(" name          : %s\n", inode.name);
        printf(" size          : %u bytes\n", inode.size);
        // выводим только реальные прямые блоки:
        unsigned int blocks_used = (inode.size + BLOCK_SIZE - 1) / BLOCK_SIZE;
        printf(" direct_blocks :");
        for (unsigned int j = 0; j < blocks_used && j < MAX_BLOCKS_PER_FILE; ++j)
        {
            printf(" %u", inode.direct_blocks[j]);
        }
        printf("\n");
    }

    if (!any_printed)
    {
        printf(WARNING "" BOLD " В таблице Inode нет ни одного занятого entry — все Inode пустые." RESET "\n");
    }
}
