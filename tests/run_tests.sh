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
    -Wl,--wrap=calloc -Wl,--wrap=realloc \
    -o navigation_tests
./navigation_tests
gcc menu_load_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw \
    -Wl,--wrap=fm_switch -Wl,--wrap=fm_add -Wl,--wrap=update_status_bar \
    -Wl,--wrap=confirm_switch -Wl,--wrap=allocation_failed -Wl,--wrap=clamp_scroll_x \
    -Wl,--wrap=draw_text_buffer -Wl,--wrap=redraw -Wl,--wrap=strdup \
    -Wl,--wrap=create_popup_window -Wl,--wrap=curs_set -Wl,--wrap=mvwin -Wl,--wrap=wresize \
    -Wl,--wrap=calloc -Wl,--wrap=realloc \
    -o menu_load_tests
./menu_load_tests

gcc macro_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw \
    -Wl,--wrap=fm_switch -Wl,--wrap=fm_add -Wl,--wrap=update_status_bar \
    -Wl,--wrap=confirm_switch -Wl,--wrap=allocation_failed -Wl,--wrap=clamp_scroll_x \
    -Wl,--wrap=draw_text_buffer -Wl,--wrap=redraw -Wl,--wrap=strdup \
    -Wl,--wrap=create_popup_window -Wl,--wrap=curs_set -Wl,--wrap=mvwin -Wl,--wrap=wresize \
    -Wl,--wrap=calloc -Wl,--wrap=realloc \
    -o macro_tests
./macro_tests
gcc editor_actions_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw \
    -Wl,--wrap=fm_switch -Wl,--wrap=fm_add -Wl,--wrap=update_status_bar \
    -Wl,--wrap=confirm_switch -Wl,--wrap=allocation_failed -Wl,--wrap=clamp_scroll_x \
    -Wl,--wrap=draw_text_buffer -Wl,--wrap=redraw -Wl,--wrap=strdup \
    -Wl,--wrap=create_popup_window -Wl,--wrap=curs_set -Wl,--wrap=mvwin -Wl,--wrap=wresize \
    -Wl,--wrap=calloc -Wl,--wrap=realloc \
    -o editor_actions_tests
./editor_actions_tests
gcc clipboard_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw \
    -Wl,--wrap=fm_switch -Wl,--wrap=fm_add -Wl,--wrap=update_status_bar \
    -Wl,--wrap=confirm_switch -Wl,--wrap=allocation_failed -Wl,--wrap=clamp_scroll_x \
    -Wl,--wrap=draw_text_buffer -Wl,--wrap=redraw -Wl,--wrap=strdup \
    -Wl,--wrap=create_popup_window -Wl,--wrap=curs_set -Wl,--wrap=mvwin -Wl,--wrap=wresize \
    -Wl,--wrap=calloc -Wl,--wrap=realloc \
    -o clipboard_tests
./clipboard_tests
gcc theme_fd_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw \
    -Wl,--wrap=fm_switch -Wl,--wrap=fm_add -Wl,--wrap=update_status_bar \
    -Wl,--wrap=confirm_switch -Wl,--wrap=allocation_failed -Wl,--wrap=clamp_scroll_x \
    -Wl,--wrap=draw_text_buffer -Wl,--wrap=redraw -Wl,--wrap=strdup -Wl,--wrap=wgetch \
    -Wl,--wrap=create_popup_window -Wl,--wrap=curs_set -Wl,--wrap=mvwin -Wl,--wrap=wresize \
    -Wl,--wrap=calloc -Wl,--wrap=realloc \
    -o theme_fd_tests
./theme_fd_tests
gcc search_replace_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw \
    -Wl,--wrap=fm_switch -Wl,--wrap=fm_add -Wl,--wrap=update_status_bar \
    -Wl,--wrap=confirm_switch -Wl,--wrap=allocation_failed -Wl,--wrap=clamp_scroll_x \
    -Wl,--wrap=draw_text_buffer -Wl,--wrap=redraw -Wl,--wrap=strdup \
    -Wl,--wrap=create_popup_window -Wl,--wrap=curs_set -Wl,--wrap=mvwin -Wl,--wrap=wresize \
    -Wl,--wrap=calloc -Wl,--wrap=realloc \
    -o search_replace_tests
./search_replace_tests
gcc search_lazy_load_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw \
    -Wl,--wrap=fm_switch -Wl,--wrap=fm_add -Wl,--wrap=update_status_bar \
    -Wl,--wrap=confirm_switch -Wl,--wrap=allocation_failed -Wl,--wrap=clamp_scroll_x \
    -Wl,--wrap=draw_text_buffer -Wl,--wrap=redraw -Wl,--wrap=strdup \
    -Wl,--wrap=create_popup_window -Wl,--wrap=curs_set -Wl,--wrap=mvwin -Wl,--wrap=wresize \
    -Wl,--wrap=calloc -Wl,--wrap=realloc \
    -o search_lazy_load_tests
./search_lazy_load_tests
gcc dialog_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw \
    -Wl,--wrap=fm_switch -Wl,--wrap=fm_add -Wl,--wrap=update_status_bar \
    -Wl,--wrap=confirm_switch -Wl,--wrap=allocation_failed -Wl,--wrap=clamp_scroll_x \
    -Wl,--wrap=draw_text_buffer -Wl,--wrap=redraw -Wl,--wrap=strdup \
    -Wl,--wrap=create_popup_window -Wl,--wrap=curs_set -Wl,--wrap=mvwin -Wl,--wrap=wresize \
    -Wl,--wrap=calloc -Wl,--wrap=realloc \
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
    -Wl,--wrap=calloc -Wl,--wrap=realloc \
    -o file_dialog_resize_tests
./file_dialog_resize_tests
gcc undo_redo_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw \
    -Wl,--wrap=fm_switch -Wl,--wrap=fm_add -Wl,--wrap=update_status_bar \
    -Wl,--wrap=confirm_switch -Wl,--wrap=allocation_failed -Wl,--wrap=clamp_scroll_x \
    -Wl,--wrap=draw_text_buffer -Wl,--wrap=redraw -Wl,--wrap=strdup \
    -Wl,--wrap=create_popup_window -Wl,--wrap=curs_set -Wl,--wrap=mvwin -Wl,--wrap=wresize \
    -Wl,--wrap=calloc -Wl,--wrap=realloc \
    -o undo_redo_tests
./undo_redo_tests
gcc delete_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw \
    -Wl,--wrap=fm_switch -Wl,--wrap=fm_add -Wl,--wrap=update_status_bar \
    -Wl,--wrap=confirm_switch -Wl,--wrap=allocation_failed -Wl,--wrap=clamp_scroll_x \
    -Wl,--wrap=draw_text_buffer -Wl,--wrap=redraw -Wl,--wrap=strdup \
    -Wl,--wrap=create_popup_window -Wl,--wrap=curs_set -Wl,--wrap=mvwin -Wl,--wrap=wresize \
    -Wl,--wrap=calloc -Wl,--wrap=realloc \
    -o delete_tests
./delete_tests
gcc json_highlight_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw \
    -Wl,--wrap=fm_switch -Wl,--wrap=fm_add -Wl,--wrap=update_status_bar \
    -Wl,--wrap=confirm_switch -Wl,--wrap=allocation_failed -Wl,--wrap=clamp_scroll_x \
    -Wl,--wrap=draw_text_buffer -Wl,--wrap=redraw -Wl,--wrap=strdup \
    -Wl,--wrap=create_popup_window -Wl,--wrap=curs_set -Wl,--wrap=mvwin -Wl,--wrap=wresize \
    -Wl,--wrap=calloc -Wl,--wrap=realloc \
    -o json_highlight_tests
./json_highlight_tests
gcc mouse_drag_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw \
    -Wl,--wrap=fm_switch -Wl,--wrap=fm_add -Wl,--wrap=update_status_bar \
    -Wl,--wrap=confirm_switch -Wl,--wrap=allocation_failed -Wl,--wrap=clamp_scroll_x \
    -Wl,--wrap=draw_text_buffer -Wl,--wrap=redraw -Wl,--wrap=strdup \
    -Wl,--wrap=create_popup_window -Wl,--wrap=curs_set -Wl,--wrap=mvwin -Wl,--wrap=wresize \
    -Wl,--wrap=calloc -Wl,--wrap=realloc \
    -o mouse_drag_tests
./mouse_drag_tests
gcc line_capacity_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw \
    -Wl,--wrap=fm_switch -Wl,--wrap=fm_add -Wl,--wrap=update_status_bar \
    -Wl,--wrap=confirm_switch -Wl,--wrap=allocation_failed -Wl,--wrap=clamp_scroll_x \
    -Wl,--wrap=draw_text_buffer -Wl,--wrap=redraw -Wl,--wrap=strdup \
    -Wl,--wrap=create_popup_window -Wl,--wrap=curs_set -Wl,--wrap=mvwin -Wl,--wrap=wresize \
    -Wl,--wrap=calloc -Wl,--wrap=realloc \
    -o line_capacity_tests
./line_capacity_tests
gcc utf8_word_nav_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw \
    -Wl,--wrap=fm_switch -Wl,--wrap=fm_add -Wl,--wrap=update_status_bar \
    -Wl,--wrap=confirm_switch -Wl,--wrap=allocation_failed -Wl,--wrap=clamp_scroll_x \
    -Wl,--wrap=draw_text_buffer -Wl,--wrap=redraw -Wl,--wrap=strdup \
    -Wl,--wrap=create_popup_window -Wl,--wrap=curs_set -Wl,--wrap=mvwin -Wl,--wrap=wresize \
    -Wl,--wrap=calloc -Wl,--wrap=realloc \
    -o utf8_word_nav_tests
./utf8_word_nav_tests
gcc syntax_mode_tests.c obj_test/test_stubs.o -I$SRC -Lobj_test -lvento -lncursesw \
    -Wl,--wrap=fm_switch -Wl,--wrap=fm_add -Wl,--wrap=update_status_bar \
    -Wl,--wrap=confirm_switch -Wl,--wrap=allocation_failed -Wl,--wrap=clamp_scroll_x \
    -Wl,--wrap=draw_text_buffer -Wl,--wrap=redraw -Wl,--wrap=strdup \
    -Wl,--wrap=create_popup_window -Wl,--wrap=curs_set -Wl,--wrap=mvwin -Wl,--wrap=wresize \
    -Wl,--wrap=calloc -Wl,--wrap=realloc \
    -o syntax_mode_tests
./syntax_mode_tests
