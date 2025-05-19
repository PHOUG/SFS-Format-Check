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

int format_disk()
{
    system("clear");
    char disk_name[256];
    printf(ENTER "" BOLD " Введите название" RESET ": ");
    scanf("%s", disk_name);

    // Проверка существования файла
    FILE *check = fopen(disk_name, "rb");
    if (check)
    {
        char answer;
        do
        {
            printf("\n" INFO "" BOLD " Файл уже " RESET "%s" BOLD " существует" RESET ". Выполнить форматирование? [y/n]: ", disk_name);
            scanf(" %c", &answer);

            switch (answer)
            {
            case 'y':
            case 'Y':
                // Пользователь согласился — просто выходим из цикла и продолжаем форматирование
                break;
            case 'n':
            case 'N':
                // Пользователь отказался — отменяем операцию
                printf(INFO "" BOLD " Форматирование отменено" RESET ": %s не будет перезаписан.\n\n", disk_name);
                fclose(check);
                return 0;
            default:
                // Неверный ввод — просим повторить
                printf(WARNING "" BOLD " Неверный ввод" RESET ". Пожалуйста, повторите.\n");
                continue;
            }

            // Если дошли до этого места, значит был введён 'y' или 'Y'
            break;

        } while (1);
        fclose(check);
    }
    else
    {
        // Файл не существует, продолжаем форматирование
        printf(INFO "" BOLD " Файл не найден" RESET ": будет создан новый файл %s.\n", disk_name);
    }

    printf("\n" PROCESS "" BOLD " Форматирование" RESET ": открытие файла %s для записи...\n", disk_name);
    FILE *disk = fopen(disk_name, "wb");
    if (!disk)
    {
        perror("fopen");
        return 1;
    }

    fseek(disk, DISK_SIZE - 1, SEEK_SET);
    fputc('\0', disk);

    fseek(disk, 0, SEEK_SET);

    printf("" PROCESS "" BOLD " Форматирование" RESET ": создание суперблока, битовой карты и inode-таблицы...\n");
    printf("" PROCESS "" BOLD " Форматирование" RESET ": создание суперблока...\n");
    write_superblock(disk);
    printf("" PROCESS "" BOLD " Форматирование" RESET ": создание битовой карты...\n");
    write_bitmap(disk);
    printf("" PROCESS "" BOLD " Форматирование" RESET ": создание inode-таблицы...\n");
    write_inodes(disk);

    fclose(disk);
    printf("" OK "" BOLD " Форматирование завершено" RESET ": %s готов к использованию.\n\n", disk_name);
    return 0;
}

// ==== Function Realization ====

void write_superblock(FILE *disk)
{
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

void write_bitmap(FILE *disk)
{
    uint8_t bitmap[BLOCK_SIZE * BITMAP_BLOCK_COUNT] = {0};

    // Пометить служебные блоки как занятые
    for (int i = 0; i < DATA_BLOCK_START; ++i)
    {
        bitmap[i / 8] |= (1 << (i % 8));
    }

    fseek(disk, BITMAP_BLOCK_START * BLOCK_SIZE, SEEK_SET);
    fwrite(bitmap, sizeof(bitmap), 1, disk);
}

void write_inodes(FILE *disk)
{
    Inode inode = {0}; // все inodes пустые
    fseek(disk, INODE_BLOCK_START * BLOCK_SIZE, SEEK_SET);
    for (unsigned int i = 0; i < MAX_INODES; ++i)
    {
        fwrite(&inode, sizeof(Inode), 1, disk);
    }
}
