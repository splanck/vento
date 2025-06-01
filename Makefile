CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L

ifeq ($(shell uname),Darwin)
    CURSES_LIB = -lncurses
    CFLAGS += -D_XOPEN_SOURCE_EXTENDED -D_DARWIN_C_SOURCE
else
    CURSES_LIB = -lncursesw
endif

SRC_DIR = ./src
OBJ_DIR = ./obj
BIN_DIR = ./bin
DOC_DIR = ./docs

# Include all source files. Explicitly list new line buffer module so
# builds pick it up even if wildcard caching occurs.
SRCS := $(sort $(wildcard $(SRC_DIR)/*.c) $(SRC_DIR)/line_buffer.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
DEPS = $(wildcard $(SRC_DIR)/*.h)
MANPAGE = $(DOC_DIR)/vento.1

$(BIN_DIR)/vento: $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(CURSES_LIB)

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

# Runs the unit tests
test:
	sh tests/run_tests.sh
