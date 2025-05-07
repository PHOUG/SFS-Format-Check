CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude

SRC_DIR = src
BIN_FORMAT = exec/format
BIN_CHECK = exec/check

all: $(BIN_FORMAT) $(BIN_CHECK)

$(BIN_FORMAT): $(SRC_DIR)/format.c
	$(CC) $(CFLAGS) -o $(BIN_FORMAT) $(SRC_DIR)/format.c

$(BIN_CHECK): $(SRC_DIR)/check.c
	$(CC) $(CFLAGS) -o $(BIN_CHECK) $(SRC_DIR)/check.c

clean:
	rm -f $(BIN_FORMAT) $(BIN_CHECK) disk.img
