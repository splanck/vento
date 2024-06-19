CC = gcc
CFLAGS = -Wall -Wextra -std=c99

OBJS = vento.o editor.o input.o
DEPS = editor.h input.h

vento: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lncurses

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) vento
