#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "define.h"
#include "structures.h"
#include "colors.h"

void clear_screen();
int open_disk(FILE **disk);
void corrupt_superblock(FILE *disk);
void corrupt_bitmap(FILE *disk);
void corrupt_inode(FILE *disk);
void corrupt_block_entry(FILE *disk);

int break_disk(void)
{
    clear_screen();
    printf("SFS Break Utility — интерактивное порча компонентов диска\n");
    printf("Выберите опцию:\n");
    printf("  1. Повредить superblock (magic)\n");
    printf("  2. Перевернуть бит в bitmap\n");
    printf("  3. Обнулить поле used в inode\n");
    printf("  4. Записать некорректный номер блока в inode.direct_blocks\n");
    printf("  5. Выход\n");
    printf("Ваш выбор: ");

    int choice;
    if (scanf("%d", &choice) != 1)
    {
        printf(ERROR " Неверный ввод\n");
        return 1;
    }

    if (choice == 5)
    {
        printf("Выход...\n");
        return 0;
    }

    FILE *disk;
    if (open_disk(&disk) != 0)
    {
        return 1;
    }

    switch (choice)
    {
    case 1:
        corrupt_superblock(disk);
        break;
    case 2:
        corrupt_bitmap(disk);
        break;
    case 3:
        corrupt_inode(disk);
        break;
    case 4:
        corrupt_block_entry(disk);
        break;
    default:
        printf(ERROR " Неверная опция\n");
    }

    fclose(disk);
    return 0;
}

void clear_screen()
{
    system("clear");
}

int open_disk(FILE **disk)
{
    char disk_name[256];
    printf(ENTER "%s Введите имя файла-диска" RESET ": ", "");
    if (scanf("%255s", disk_name) != 1)
    {
        printf(ERROR "%s Неверный ввод\n", "");
        return -1;
    }
    *disk = fopen(disk_name, "r+b");
    if (!*disk)
    {
        printf(ERROR "%s Не удалось открыть диск %s\n", "", disk_name);
        return -1;
    }
    return 0;
}

void corrupt_superblock(FILE *disk)
{
    SuperBlock sb;
    fseek(disk, SUPERBLOCK_BLOCK * BLOCK_SIZE, SEEK_SET);
    fread(&sb, sizeof(sb), 1, disk);
    printf(INFO " Старый magic: 0x%X\n", sb.magic);
    sb.magic = 0xDEADBEEF;
    fseek(disk, SUPERBLOCK_BLOCK * BLOCK_SIZE, SEEK_SET);
    fwrite(&sb, sizeof(sb), 1, disk);
    fflush(disk);
    fsync(fileno(disk));
    printf(SUCCESS " Magic в суперблоке изменён на 0x%X\n", sb.magic);
}

void corrupt_bitmap(FILE *disk)
{
    int blk;
    printf(ENTER "%s Введите индекс блока для переворота бита" RESET ": ", "");
    scanf("%d", &blk);
    if (blk < 0 || blk >= TOTAL_BLOCKS)
    {
        printf(ERROR " Неверный индекс блока: %d\n", blk);
        return;
    }
    size_t bmp_bytes = BITMAP_BLOCK_COUNT * BLOCK_SIZE;
    uint8_t *bitmap = malloc(bmp_bytes);
    fseek(disk, BITMAP_BLOCK_START * BLOCK_SIZE, SEEK_SET);
    fread(bitmap, 1, bmp_bytes, disk);
    int bit = (bitmap[blk / 8] >> (blk % 8)) & 1;
    printf(INFO " До: блок %d в bitmap = %d\n", blk, bit);
    bitmap[blk / 8] ^= (1 << (blk % 8));
    fseek(disk, BITMAP_BLOCK_START * BLOCK_SIZE, SEEK_SET);
    fwrite(bitmap, 1, bmp_bytes, disk);
    fflush(disk);
    fsync(fileno(disk));
    printf(SUCCESS " После: блок %d в bitmap = %d\n", blk,
           (bitmap[blk / 8] >> (blk % 8)) & 1);
    free(bitmap);
}

void corrupt_inode(FILE *disk)
{
    unsigned long idx;
    printf(ENTER "%s Введите индекс inode для обнуления used" RESET ": ", "");
    scanf("%lu", &idx);
    if (idx < 0 || idx >= MAX_INODES)
    {
        printf(ERROR " Неверный индекс inode: %lu\n", idx);
        return;
    }
    Inode inode;
    long pos = INODE_BLOCK_START * BLOCK_SIZE + idx * sizeof(Inode);
    fseek(disk, pos, SEEK_SET);
    fread(&inode, sizeof(inode), 1, disk);
    printf(INFO " Inode #%lu: поле used = %d\n", idx, inode.used);
    inode.used = 0;
    fseek(disk, pos, SEEK_SET);
    fwrite(&inode, sizeof(inode), 1, disk);
    fflush(disk);
    fsync(fileno(disk));
    printf(SUCCESS " Inode #%lu: поле used обнулено\n", idx);
}

void corrupt_block_entry(FILE *disk)
{
    unsigned long idx, slot;
    printf(ENTER "%s Введите индекс inode и слот direct_blocks" RESET " (напр. 3 1): ", "");
    scanf("%lu %lu", &idx, &slot);
    if (idx < 0 || idx >= MAX_INODES || slot < 0 || slot >= MAX_BLOCKS_PER_FILE)
    {
        printf(ERROR " Неверные параметры inode=%lu slot=%lu\n", idx, slot);
        return;
    }
    Inode inode;
    long pos = INODE_BLOCK_START * BLOCK_SIZE + idx * sizeof(Inode);
    fseek(disk, pos, SEEK_SET);
    fread(&inode, sizeof(inode), 1, disk);
    printf(INFO " Inode #%lu: direct_blocks[%lu] = %u\n", idx, slot,
           inode.direct_blocks[slot]);
    inode.direct_blocks[slot] = TOTAL_BLOCKS + 100;
    fseek(disk, pos, SEEK_SET);
    fwrite(&inode, sizeof(inode), 1, disk);
    fflush(disk);
    fsync(fileno(disk));
    printf(SUCCESS " Inode #%lu: direct_blocks[%lu] испорчен\n", idx, slot);
}