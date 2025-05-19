#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "colors.h"
#include "format.h" // Подключение заголовочного файла для format_disk
#include "check.h"  // Подключение заголовочного файла для check_main
#include "dump.h"   // Подключение заголовочного файла для dump_file
#include "fill.h"   // Заполнение файла-диска
#include "break.h"

void print_help();

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("\n" ERROR " Не указана команда\n");
        print_help();
        return 1;
    }

    if (strcmp(argv[1], "format") == 0)
    {
        return format_disk();
    }
    else if (strcmp(argv[1], "check") == 0)
    {
        return check_main();
    }
    if (strcmp(argv[1], "dump") == 0)
    {
        return dump_file();
    }
    if (strcmp(argv[1], "fill") == 0)
    {
        return fill_file();
    }
        if (strcmp(argv[1], "break") == 0)
    {
        return break_disk();
    }
    else if (strcmp(argv[1], "help") == 0)
    {
        print_help();
        return 0;
    }
    else
    {
        printf("" ERROR " Неизвестная команда: %s\n", argv[1]);
        print_help();
        return 1;
    }

    return 0;
}

void print_help()
{
    printf("SFS Utility — форматирование и проверка файловой системы\n");
    printf("Использование:\n");
    printf(" > " ITALIC ".exec/sfs format" RESET " — форматировать виртуальный диск\n");
    printf(" > " ITALIC ".exec/sfs check" RESET "  — проверить целостность файловой системы\n");
    printf(" > " ITALIC ".exec/sfs dump" RESET "  — вывести содержимое диска\n");
    printf(" > " ITALIC ".exec/sfs fill" RESET "  — заполнить файл-диск данными\n");
    printf(" > " ITALIC ".exec/sfs break" RESET "  — испортить файл-диск для тестов\n");
    printf(" > " ITALIC ".exec/sfs help" RESET "   — вывести эту справку\n\n");
}