#!/bin/sh
set -e
# Ensure a clean build directory
rm -rf obj_test
mkdir obj_test
# compile minimal sources needed for the test
for src in src/clipboard.c src/files.c; do
    gcc -Wall -Wextra -std=c99 -g -Isrc -c "$src" -o obj_test/$(basename "$src" .c).o
done
# compile test file with stubbed insert_new_line
gcc -Wall -Wextra -std=c99 -g -Isrc tests/test_paste.c obj_test/*.o -lncurses -o test_paste
./test_paste
