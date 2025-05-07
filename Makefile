# Компилятор и флаги
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -Iinclude

# Директории
SRC_DIR = src
INC_DIR = include
OBJ_DIR = build
BIN_DIR = exec
BIN = $(BIN_DIR)/sfs

# Исходники и объектные файлы
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Цель по умолчанию
all: $(BIN)

# Сборка исполняемого файла
$(BIN): $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

# Сборка object-файлов
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Создание директорий
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Очистка всех сгенерированных файлов
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Запуск программы
run: all
	./$(BIN)

.PHONY: all clean run
