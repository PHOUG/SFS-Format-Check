#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "colors.h"
#include "format.h"  // Подключение заголовочного файла для format_disk
#include "check.h"   // Подключение заголовочного файла для check_main

void print_help();

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("\n["RED""BOLD"ERROR"RESET"] Не указана команда\n");
        print_help();
        return 1;
    }

    if (strcmp(argv[1], "format") == 0) {
        format_disk();
    } else if (strcmp(argv[1], "check") == 0) {
        return check_main();
    } else if (strcmp(argv[1], "help") == 0) {
        print_help();
    } else {
        printf("["RED""BOLD"ERROR"RESET"] Неизвестная команда: %s\n", argv[1]);
        print_help();
        return 1;
    }

    return 0;
}

void print_help() {
    printf("SFS Utility — форматирование и проверка файловой системы\n");
    printf("Использование:\n");
    printf(" > "ITALIC"./sfs format"RESET" — форматировать виртуальный диск\n");
    printf(" > "ITALIC"./sfs check"RESET"  — проверить целостность файловой системы\n");
    printf(" > "ITALIC"./sfs help"RESET"   — вывести эту справку\n\n");
}