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
    -Wl,--wrap=create_popup_window -Wl,--wrap=curs_set -Wl,--wrap=mvwin -Wl,--wrap=wresize \
    -o navigation_tests
./navigation_tests
gcc menu_load_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw \
    -Wl,--wrap=fm_switch -Wl,--wrap=fm_add -Wl,--wrap=update_status_bar \
    -Wl,--wrap=confirm_switch -Wl,--wrap=allocation_failed -Wl,--wrap=clamp_scroll_x \
    -Wl,--wrap=draw_text_buffer -Wl,--wrap=redraw -Wl,--wrap=strdup \
    -Wl,--wrap=create_popup_window -Wl,--wrap=curs_set -Wl,--wrap=mvwin -Wl,--wrap=wresize \
    -o menu_load_tests
./menu_load_tests

gcc macro_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw \
    -Wl,--wrap=fm_switch -Wl,--wrap=fm_add -Wl,--wrap=update_status_bar \
    -Wl,--wrap=confirm_switch -Wl,--wrap=allocation_failed -Wl,--wrap=clamp_scroll_x \
    -Wl,--wrap=draw_text_buffer -Wl,--wrap=redraw -Wl,--wrap=strdup \
    -Wl,--wrap=create_popup_window -Wl,--wrap=curs_set -Wl,--wrap=mvwin -Wl,--wrap=wresize \
    -o macro_tests
./macro_tests
gcc editor_actions_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw \
    -Wl,--wrap=fm_switch -Wl,--wrap=fm_add -Wl,--wrap=update_status_bar \
    -Wl,--wrap=confirm_switch -Wl,--wrap=allocation_failed -Wl,--wrap=clamp_scroll_x \
    -Wl,--wrap=draw_text_buffer -Wl,--wrap=redraw -Wl,--wrap=strdup \
    -Wl,--wrap=create_popup_window -Wl,--wrap=curs_set -Wl,--wrap=mvwin -Wl,--wrap=wresize \
    -o editor_actions_tests
./editor_actions_tests
gcc clipboard_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw \
    -Wl,--wrap=fm_switch -Wl,--wrap=fm_add -Wl,--wrap=update_status_bar \
    -Wl,--wrap=confirm_switch -Wl,--wrap=allocation_failed -Wl,--wrap=clamp_scroll_x \
    -Wl,--wrap=draw_text_buffer -Wl,--wrap=redraw -Wl,--wrap=strdup \
    -Wl,--wrap=create_popup_window -Wl,--wrap=curs_set -Wl,--wrap=mvwin -Wl,--wrap=wresize \
    -o clipboard_tests
./clipboard_tests
gcc theme_fd_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw \
    -Wl,--wrap=fm_switch -Wl,--wrap=fm_add -Wl,--wrap=update_status_bar \
    -Wl,--wrap=confirm_switch -Wl,--wrap=allocation_failed -Wl,--wrap=clamp_scroll_x \
    -Wl,--wrap=draw_text_buffer -Wl,--wrap=redraw -Wl,--wrap=strdup -Wl,--wrap=wgetch \
    -Wl,--wrap=create_popup_window -Wl,--wrap=curs_set -Wl,--wrap=mvwin -Wl,--wrap=wresize \
    -o theme_fd_tests
./theme_fd_tests
gcc search_replace_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw \
    -Wl,--wrap=fm_switch -Wl,--wrap=fm_add -Wl,--wrap=update_status_bar \
    -Wl,--wrap=confirm_switch -Wl,--wrap=allocation_failed -Wl,--wrap=clamp_scroll_x \
    -Wl,--wrap=draw_text_buffer -Wl,--wrap=redraw -Wl,--wrap=strdup \
    -Wl,--wrap=create_popup_window -Wl,--wrap=curs_set -Wl,--wrap=mvwin -Wl,--wrap=wresize \
    -o search_replace_tests
./search_replace_tests
gcc dialog_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw \
    -Wl,--wrap=fm_switch -Wl,--wrap=fm_add -Wl,--wrap=update_status_bar \
    -Wl,--wrap=confirm_switch -Wl,--wrap=allocation_failed -Wl,--wrap=clamp_scroll_x \
    -Wl,--wrap=draw_text_buffer -Wl,--wrap=redraw -Wl,--wrap=strdup \
    -Wl,--wrap=create_popup_window -Wl,--wrap=curs_set -Wl,--wrap=mvwin -Wl,--wrap=wresize \
    -o dialog_tests
./dialog_tests
gcc goto_dialog_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw \
    -Wl,--wrap=fm_switch -Wl,--wrap=fm_add -Wl,--wrap=update_status_bar \
    -Wl,--wrap=confirm_switch -Wl,--wrap=allocation_failed -Wl,--wrap=clamp_scroll_x \
    -Wl,--wrap=draw_text_buffer -Wl,--wrap=redraw -Wl,--wrap=strdup \
    -Wl,--wrap=create_popup_window -Wl,--wrap=curs_set -Wl,--wrap=create_dialog \
    -o goto_dialog_tests
./goto_dialog_tests
gcc help_text_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw \
    -Wl,--wrap=fm_switch -Wl,--wrap=fm_add -Wl,--wrap=update_status_bar \
    -Wl,--wrap=confirm_switch -Wl,--wrap=allocation_failed -Wl,--wrap=clamp_scroll_x \
    -Wl,--wrap=draw_text_buffer -Wl,--wrap=redraw -Wl,--wrap=strdup \
    -Wl,--wrap=create_popup_window -Wl,--wrap=curs_set -Wl,--wrap=create_dialog \
    -Wl,--wrap=mvprintw \
    -o help_text_tests
./help_text_tests
gcc file_dialog_resize_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw \
    -Wl,--wrap=fm_switch -Wl,--wrap=fm_add -Wl,--wrap=update_status_bar \
    -Wl,--wrap=confirm_switch -Wl,--wrap=allocation_failed -Wl,--wrap=clamp_scroll_x \
    -Wl,--wrap=draw_text_buffer -Wl,--wrap=redraw -Wl,--wrap=strdup \
    -Wl,--wrap=create_popup_window -Wl,--wrap=curs_set -Wl,--wrap=mvwin -Wl,--wrap=wresize \
    -o file_dialog_resize_tests
./file_dialog_resize_tests
gcc undo_redo_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw \
    -Wl,--wrap=fm_switch -Wl,--wrap=fm_add -Wl,--wrap=update_status_bar \
    -Wl,--wrap=confirm_switch -Wl,--wrap=allocation_failed -Wl,--wrap=clamp_scroll_x \
    -Wl,--wrap=draw_text_buffer -Wl,--wrap=redraw -Wl,--wrap=strdup \
    -Wl,--wrap=create_popup_window -Wl,--wrap=curs_set -Wl,--wrap=mvwin -Wl,--wrap=wresize \
    -o undo_redo_tests
./undo_redo_tests
gcc json_highlight_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw \
    -Wl,--wrap=fm_switch -Wl,--wrap=fm_add -Wl,--wrap=update_status_bar \
    -Wl,--wrap=confirm_switch -Wl,--wrap=allocation_failed -Wl,--wrap=clamp_scroll_x \
    -Wl,--wrap=draw_text_buffer -Wl,--wrap=redraw -Wl,--wrap=strdup \
    -Wl,--wrap=create_popup_window -Wl,--wrap=curs_set -Wl,--wrap=mvwin -Wl,--wrap=wresize \
    -o json_highlight_tests
./json_highlight_tests
