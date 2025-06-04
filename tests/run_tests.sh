#!/bin/sh
set -e
DIR="$(dirname "$0")"
cd "$DIR"
mkdir -p obj_test
SRC="../src"
for f in $SRC/*.c; do
    bn=$(basename "$f")
    if [ "$bn" != "vento.c" ]; then
        gcc -c "$f" -o obj_test/$(basename "$f" .c).o -I$SRC \
            -D_POSIX_C_SOURCE=200809L -std=c99 -Wall -Wextra -fcommon
    fi
done
ar rcs obj_test/libvento.a obj_test/*.o
gcc -c test_stubs.c -I$SRC -o obj_test/test_stubs.o
gcc navigation_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw \
    -Wl,--wrap=fm_switch -Wl,--wrap=fm_add -Wl,--wrap=update_status_bar \
    -Wl,--wrap=confirm_switch -Wl,--wrap=allocation_failed -Wl,--wrap=clamp_scroll_x \
    -Wl,--wrap=draw_text_buffer -Wl,--wrap=redraw -Wl,--wrap=strdup \
    -o navigation_tests
./navigation_tests
gcc menu_load_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw \
    -Wl,--wrap=fm_switch -Wl,--wrap=fm_add -Wl,--wrap=update_status_bar \
    -Wl,--wrap=confirm_switch -Wl,--wrap=allocation_failed -Wl,--wrap=clamp_scroll_x \
    -Wl,--wrap=draw_text_buffer -Wl,--wrap=redraw -Wl,--wrap=strdup \
    -o menu_load_tests
./menu_load_tests

gcc macro_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw \
    -Wl,--wrap=fm_switch -Wl,--wrap=fm_add -Wl,--wrap=update_status_bar \
    -Wl,--wrap=confirm_switch -Wl,--wrap=allocation_failed -Wl,--wrap=clamp_scroll_x \
    -Wl,--wrap=draw_text_buffer -Wl,--wrap=redraw -Wl,--wrap=strdup \
    -o macro_tests
./macro_tests
gcc editor_actions_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw \
    -Wl,--wrap=fm_switch -Wl,--wrap=fm_add -Wl,--wrap=update_status_bar \
    -Wl,--wrap=confirm_switch -Wl,--wrap=allocation_failed -Wl,--wrap=clamp_scroll_x \
    -Wl,--wrap=draw_text_buffer -Wl,--wrap=redraw -Wl,--wrap=strdup \
    -o editor_actions_tests
./editor_actions_tests
gcc clipboard_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw \
    -Wl,--wrap=fm_switch -Wl,--wrap=fm_add -Wl,--wrap=update_status_bar \
    -Wl,--wrap=confirm_switch -Wl,--wrap=allocation_failed -Wl,--wrap=clamp_scroll_x \
    -Wl,--wrap=draw_text_buffer -Wl,--wrap=redraw -Wl,--wrap=strdup \
    -o clipboard_tests
./clipboard_tests
gcc theme_fd_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw \
    -Wl,--wrap=fm_switch -Wl,--wrap=fm_add -Wl,--wrap=update_status_bar \
    -Wl,--wrap=confirm_switch -Wl,--wrap=allocation_failed -Wl,--wrap=clamp_scroll_x \
    -Wl,--wrap=draw_text_buffer -Wl,--wrap=redraw -Wl,--wrap=strdup -Wl,--wrap=wgetch \
    -o theme_fd_tests
./theme_fd_tests
gcc search_replace_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw \
    -Wl,--wrap=fm_switch -Wl,--wrap=fm_add -Wl,--wrap=update_status_bar \
    -Wl,--wrap=confirm_switch -Wl,--wrap=allocation_failed -Wl,--wrap=clamp_scroll_x \
    -Wl,--wrap=draw_text_buffer -Wl,--wrap=redraw -Wl,--wrap=strdup \
    -o search_replace_tests
./search_replace_tests
