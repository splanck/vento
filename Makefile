CC = gcc
CFLAGS = -Wall -Wextra -std=c99

SRC_DIR = ./src
OBJ_DIR = ./obj
BIN_DIR = ./bin

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
DEPS = $(wildcard $(SRC_DIR)/*.h)

$(BIN_DIR)/vento: $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ -lncurses

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

install: $(BIN_DIR)/vento
	install -d $(PREFIX)/bin
	install -m 755 $(BIN_DIR)/vento $(PREFIX)/bin

uninstall:
	rm -f $(PREFIX)/bin/vento

clean:
	rm -f $(OBJ_DIR)/*.o $(BIN_DIR)/vento
