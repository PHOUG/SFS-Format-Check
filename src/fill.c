#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "define.h"
#include "structures.h"
#include "colors.h"

int fill_file(void)
{
    system("clear");

    char disk_name[256];
    printf(ENTER "" BOLD " Введите имя файла-диска" RESET ": ");
    if (scanf("%255s", disk_name) != 1)
    {
        printf(ERROR "" BOLD " Неверный ввод\n");
        return 1;
    }

    FILE *disk = fopen(disk_name, "r+b");
    if (!disk)
    {
        printf(ERROR "" BOLD " Не удалось открыть диск %s\n", disk_name);
        return 1;
    }

    // 1) Считываем bitmap
    size_t bmp_bytes = BITMAP_BLOCK_COUNT * BLOCK_SIZE;
    uint8_t *bitmap = malloc(bmp_bytes);
    if (!bitmap)
    {
        printf(ERROR "" BOLD " Нет памяти для bitmap\n");
        fclose(disk);
        return 1;
    }
    if (fseek(disk, BITMAP_BLOCK_START * BLOCK_SIZE, SEEK_SET) != 0 ||
        fread(bitmap, 1, bmp_bytes, disk) != bmp_bytes)
    {
        printf(ERROR "" BOLD " Ошибка чтения bitmap\n");
        free(bitmap);
        fclose(disk);
        return 1;
    }

    // 2) Подготавливаем буфер блока
    uint8_t *block_buf = malloc(BLOCK_SIZE);
    if (!block_buf)
    {
        printf(ERROR "" BOLD " Нет памяти для буфера блока\n");
        free(bitmap);
        fclose(disk);
        return 1;
    }
    memset(block_buf, 0xAA, BLOCK_SIZE);

    size_t filled = 0;

    // 3) Заполняем свободные inode
    for (size_t ino_idx = 0; ino_idx < MAX_INODES; ++ino_idx)
    {
        Inode inode;
        // читаем Inode
        fseek(disk,
              INODE_BLOCK_START * BLOCK_SIZE + ino_idx * sizeof(Inode),
              SEEK_SET);
        if (fread(&inode, sizeof(Inode), 1, disk) != 1)
        {
            printf(ERROR "" BOLD " Ошибка чтения Inode #%zu\n", ino_idx);
            break;
        }
        if (inode.used)
            continue;

        // ищем свободный блок данных
        int block_found = -1;
        for (unsigned int b = DATA_BLOCK_START; b < TOTAL_BLOCKS; ++b)
        {
            if (!(bitmap[b / 8] & (1 << (b % 8))))
            {
                block_found = b;
                bitmap[b / 8] |= (1 << (b % 8));
                break;
            }
        }
        if (block_found < 0)
            break;

        // готовим новый Inode
        Inode new_inode = {0};
        new_inode.used = 1;
        snprintf(new_inode.name, MAX_FILENAME_LENGTH, "fill%zu", filled);
        new_inode.size = BLOCK_SIZE;
        new_inode.direct_blocks[0] = block_found;

        // пишем Inode
        fseek(disk,
              INODE_BLOCK_START * BLOCK_SIZE + ino_idx * sizeof(Inode),
              SEEK_SET);
        fwrite(&new_inode, sizeof(Inode), 1, disk);
        fflush(disk);
        fsync(fileno(disk));

        // пишем данные
        fseek(disk, block_found * BLOCK_SIZE, SEEK_SET);
        fwrite(block_buf, 1, BLOCK_SIZE, disk);
        fflush(disk);
        fsync(fileno(disk));

        // обновляем bitmap
        fseek(disk, BITMAP_BLOCK_START * BLOCK_SIZE, SEEK_SET);
        fwrite(bitmap, 1, bmp_bytes, disk);
        fflush(disk);
        fsync(fileno(disk));

        filled++;
    }

    Inode zero = {0};
    for (size_t ino_idx = filled; ino_idx < MAX_INODES; ++ino_idx)
    {
        fseek(disk,
              INODE_BLOCK_START * BLOCK_SIZE + ino_idx * sizeof(Inode),
              SEEK_SET);
        fwrite(&zero, sizeof(zero), 1, disk);
    }
    fflush(disk);
    fsync(fileno(disk));

    printf(OK "" BOLD " Заполнено файлов: %zu\n" RESET, filled);

    free(bitmap);
    free(block_buf);
    fclose(disk);
    return 0;
}
