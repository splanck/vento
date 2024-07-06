CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g

SRC_DIR = ./src
OBJ_DIR = ./obj
BIN_DIR = ./bin
DOC_DIR = ./docs

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
DEPS = $(wildcard $(SRC_DIR)/*.h)
MANPAGE = $(DOC_DIR)/vento.1

$(BIN_DIR)/vento: $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ -lncurses

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

install: $(BIN_DIR)/vento
	install -d $(DESTDIR)/usr/local/bin
	install -m 755 $(BIN_DIR)/vento $(DESTDIR)/usr/local/bin
	install -d $(DESTDIR)/usr/share/man/man1
	install -m 644 $(MANPAGE) $(DESTDIR)/usr/share/man/man1

uninstall:
	rm -f $(DESTDIR)/usr/local/bin/vento
	rm -f $(DESTDIR)/usr/share/man/man1/vento.1

clean:
	rm -f $(OBJ_DIR)/*.o $(BIN_DIR)/vento
