#!/bin/sh
set -e
# Ensure a clean build directory
rm -rf obj_test
mkdir obj_test

# compile sources for paste test
gcc -Wall -Wextra -std=c99 -g -Isrc -c src/clipboard.c -o obj_test/clipboard.o
gcc -Wall -Wextra -std=c99 -g -Isrc -c src/files.c -o obj_test/files.o

# build and run paste test (provides its own stubs)
gcc -Wall -Wextra -std=c99 -g -Isrc tests/test_paste.c obj_test/clipboard.o obj_test/files.o -lncurses -o test_paste
./test_paste

# compile additional source for file state test
gcc -Wall -Wextra -std=c99 -g -Isrc -c src/file_manager.c -o obj_test/file_manager.o

# build and run file state initialization/switching test
gcc -Wall -Wextra -std=c99 -g -Isrc tests/test_file_state.c obj_test/files.o obj_test/file_manager.o -lncurses -o test_file_state
./test_file_state

# build and run resize handling test (provides many stubs)
gcc -Wall -Wextra -std=c99 -g -Isrc tests/test_resize.c obj_test/files.o obj_test/file_manager.o -lncurses -o test_resize
./test_resize

# build and run line truncation resize test
gcc -Wall -Wextra -std=c99 -g -Isrc tests/test_resize_trunc.c obj_test/files.o obj_test/file_manager.o -lncurses -o test_resize_trunc
./test_resize_trunc
