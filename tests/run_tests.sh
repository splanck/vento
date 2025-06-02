#!/bin/sh
set -e
DIR="$(dirname "$0")"
cd "$DIR"
mkdir -p obj_test
SRC="../src"
for f in $SRC/*.c; do bn=$(basename "$f"); if [ "$bn" != "vento.c" ]; then gcc -c "$f" -o obj_test/$(basename "$f" .c).o -I$SRC -D_POSIX_C_SOURCE=200809L -std=c99 -Wall -Wextra; fi; done
ar rcs obj_test/libvento.a obj_test/*.o
gcc -c test_stubs.c -I$SRC -o obj_test/test_stubs.o
gcc navigation_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw -Wl,--wrap=fm_switch -Wl,--wrap=update_status_bar -Wl,--wrap=confirm_switch -o navigation_tests
./navigation_tests
