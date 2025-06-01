#!/bin/sh
set -e
# Ensure a clean build directory
rm -rf obj_test
mkdir obj_test

# compile sources for paste test
gcc -Wall -Wextra -std=c99 -g -Isrc -c tests/stub_ui_settings_deps.c -o obj_test/stub_ui_settings_deps.o
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc -c src/clipboard.c -o obj_test/clipboard.o
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc -c src/files.c -o obj_test/files.o
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc -c src/line_buffer.c -o obj_test/line_buffer.o
gcc -Wall -Wextra -std=c99 -g -Isrc -c tests/stub_enable_color.c -o obj_test/stub_enable_color.o

# build and run paste test (provides its own stubs)
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc tests/test_paste.c obj_test/clipboard.o obj_test/files.o obj_test/line_buffer.o obj_test/stub_enable_color.o -lncursesw -o test_paste
./test_paste

# compile additional source for file state test
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc -c src/file_manager.c -o obj_test/file_manager.o
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc -c src/globals.c -o obj_test/globals.o

# build and run file state initialization/switching test
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc tests/test_file_state.c obj_test/files.o obj_test/line_buffer.o obj_test/file_manager.o obj_test/globals.o obj_test/stub_enable_color.o -lncursesw -o test_file_state
./test_file_state

# build and run resize handling test (provides many stubs)
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc tests/test_resize.c obj_test/files.o obj_test/line_buffer.o obj_test/file_manager.o obj_test/globals.o obj_test/stub_enable_color.o -lncursesw -o test_resize
./test_resize

# build and run line truncation resize test
gcc -Wall -Wextra -std=c99 -g -Isrc tests/test_resize_trunc.c obj_test/files.o obj_test/line_buffer.o obj_test/file_manager.o obj_test/globals.o obj_test/stub_enable_color.o -lncursesw -o test_resize_trunc
./test_resize_trunc

# build and run resize allocation failure test
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc tests/test_resize_allocfail.c src/line_buffer.c -lncursesw -o test_resize_allocfail
./test_resize_allocfail

# build and run resize signal handling test
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc \
    tests/test_resize_signal.c obj_test/files.o obj_test/line_buffer.o obj_test/file_manager.o obj_test/globals.o \
    obj_test/stub_enable_color.o -lncursesw -o test_resize_signal
./test_resize_signal

# build and run resize flush input test
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc \
    tests/test_resize_flushinp.c obj_test/files.o obj_test/line_buffer.o obj_test/file_manager.o obj_test/globals.o \
    obj_test/stub_enable_color.o -lncursesw -o test_resize_flushinp
./test_resize_flushinp

# build and run identifier overflow test with AddressSanitizer
gcc -Wall -Wextra -std=c99 -g -fsanitize=address -Isrc -c src/syntax_common.c -o obj_test/syntax_common.o
gcc -Wall -Wextra -std=c99 -g -Isrc -c src/syntax_regex.c -o obj_test/syntax_regex.o
gcc -Wall -Wextra -std=c99 -g -Isrc -c src/syntax_registry.c -o obj_test/syntax_registry.o
gcc -Wall -Wextra -std=c99 -g -fsanitize=address -Isrc tests/test_long_identifier.c \
    obj_test/syntax_common.o obj_test/syntax_regex.o obj_test/syntax_registry.o -lncursesw -o test_long_identifier
./test_long_identifier

# build and run HTML comment boundary test
gcc -Wall -Wextra -std=c99 -g -fsanitize=address -Isrc -c src/syntax_html.c -o obj_test/syntax_html.o
gcc -Wall -Wextra -std=c99 -g -fsanitize=address -Isrc tests/test_html_comment.c \
    obj_test/syntax_html.o obj_test/syntax_common.o obj_test/syntax_regex.o obj_test/syntax_registry.o obj_test/files.o obj_test/line_buffer.o obj_test/stub_enable_color.o -lncursesw -o test_html_comment
./test_html_comment

# build and run python syntax test
gcc -Wall -Wextra -std=c99 -g -fsanitize=address -Isrc tests/test_python_syntax.c \
    obj_test/syntax_common.o obj_test/syntax_regex.o obj_test/syntax_registry.o obj_test/files.o obj_test/line_buffer.o obj_test/stub_enable_color.o -lncursesw -o test_python_syntax
./test_python_syntax

# build and run JavaScript syntax test
gcc -Wall -Wextra -std=c99 -g -fsanitize=address -Isrc tests/test_js_syntax.c \
    obj_test/syntax_common.o obj_test/syntax_regex.o obj_test/syntax_registry.o obj_test/files.o obj_test/line_buffer.o obj_test/stub_enable_color.o -lncursesw -o test_js_syntax
./test_js_syntax

# build and run CSS syntax test
gcc -Wall -Wextra -std=c99 -g -fsanitize=address -Isrc tests/test_css_syntax.c \
    obj_test/syntax_common.o obj_test/syntax_regex.o obj_test/syntax_registry.o obj_test/files.o obj_test/line_buffer.o obj_test/stub_enable_color.o -lncursesw -o test_css_syntax
./test_css_syntax

# build and run HTML nested syntax test
gcc -Wall -Wextra -std=c99 -g -fsanitize=address -Isrc tests/test_html_nested.c \
    obj_test/syntax_html.o obj_test/syntax_common.o obj_test/syntax_regex.o obj_test/syntax_registry.o obj_test/files.o obj_test/line_buffer.o obj_test/stub_enable_color.o -lncursesw -o test_html_nested
./test_html_nested

# build and run shell syntax highlighting test
gcc -Wall -Wextra -std=c99 -g -fsanitize=address -Isrc tests/test_shell_syntax.c \
    obj_test/syntax_common.o obj_test/syntax_regex.o obj_test/syntax_registry.o obj_test/files.o obj_test/line_buffer.o obj_test/stub_enable_color.o -lncursesw -o test_shell_syntax
./test_shell_syntax

# build and run shebang detection test (uses stubs for other functions)
gcc -Wall -Wextra -std=c99 -g -Isrc -c tests/stubs_file_ops.c -o obj_test/stubs_file_ops.o
gcc -Wall -Wextra -std=c99 -g -Isrc tests/test_shebang_detection.c src/file_ops.c \
    obj_test/stubs_file_ops.o obj_test/syntax_regex.o obj_test/globals.o \
    obj_test/line_buffer.o -lncursesw -o test_shebang_detection
./test_shebang_detection

# build and run shebang case-insensitive detection test
gcc -Wall -Wextra -std=c99 -g -Isrc tests/test_shebang_case.c src/file_ops.c \
    obj_test/stubs_file_ops.o obj_test/syntax_regex.o obj_test/globals.o \
    obj_test/line_buffer.o -lncursesw -o test_shebang_case
./test_shebang_case

# build and run regex complex construct test
gcc -Wall -Wextra -std=c99 -g -fsanitize=address -Isrc tests/test_regex_complex.c \
    obj_test/syntax_common.o obj_test/syntax_regex.o obj_test/syntax_registry.o obj_test/files.o obj_test/line_buffer.o obj_test/stub_enable_color.o -lncursesw -o test_regex_complex
./test_regex_complex

# build and run search highlight test
gcc -Wall -Wextra -std=c99 -g -fsanitize=address -D_POSIX_C_SOURCE=200809L -Isrc tests/test_search_highlight.c src/search.c src/line_buffer.c -lncursesw -o test_search_highlight
./test_search_highlight

# build and run replace modified test
gcc -Wall -Wextra -std=c99 -g -fsanitize=address -D_POSIX_C_SOURCE=200809L -Isrc tests/test_replace_modified.c src/search.c src/line_buffer.c -lncursesw -o test_replace_modified
./test_replace_modified

# build and run status line clear test
gcc -Wall -Wextra -std=c99 -g tests/test_status_line_clear.c -lncursesw -o test_status_line_clear
./test_status_line_clear

# build and run undo/redo modified flag test
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc \
    tests/test_undo_redo_modified.c src/undo.c src/line_buffer.c -lncursesw -o test_undo_redo_modified
./test_undo_redo_modified

# build and run long line loading test
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc tests/test_long_line_load.c obj_test/files.o obj_test/line_buffer.o obj_test/stub_enable_color.o -lncursesw -o test_long_line_load
./test_long_line_load

# build and run long indent enter test
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc tests/test_long_indent.c src/input_keyboard.c obj_test/line_buffer.o -lncursesw -o test_long_indent
./test_long_indent

# build and run horizontal scroll test
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc \
    tests/test_horizontal_scroll.c src/input_keyboard.c obj_test/line_buffer.o -lncursesw -o test_horizontal_scroll
./test_horizontal_scroll

# build and run long horizontal scroll test
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc \
    tests/test_long_horizontal_scroll.c src/input_keyboard.c obj_test/line_buffer.o -lncursesw -o test_long_horizontal_scroll
./test_long_horizontal_scroll

# build and run color disable test
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc tests/test_color_disable.c src/globals.c -lncursesw -o test_color_disable
VENTO_THEME_DIR=./themes ./test_color_disable

# build and run initialize mouse test
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc tests/test_initialize_mouse.c -lncursesw -o test_initialize_mouse
./test_initialize_mouse

# build and run dialog color disable regression test
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc \
    -Dshow_message=stub_show_message -c tests/test_dialog_color_disable.c -o obj_test/test_dialog_color_disable.o
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc -c src/ui.c -o obj_test/ui.o
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc -c src/ui_info.c -o obj_test/ui_info.o
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc -c src/dialog.c -o obj_test/dialog.o
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc -c src/ui_common.c -o obj_test/ui_common.o
gcc obj_test/test_dialog_color_disable.o obj_test/ui.o obj_test/ui_info.o obj_test/dialog.o obj_test/ui_common.o -lncursesw -o test_dialog_color_disable
./test_dialog_color_disable

# build and run newwin failure handling test
gcc -Wall -Wextra -std=c99 -g -Isrc tests/test_newwin_fail.c src/ui_common.c -lncursesw -o test_newwin_fail
./test_newwin_fail

# build and run show_message newwin failure test
gcc -Wall -Wextra -std=c99 -g -Isrc tests/test_show_message_fail.c src/ui_common.c -lncursesw -o test_show_message_fail
./test_show_message_fail

# build and run info window creation failure test
gcc -Wall -Wextra -std=c99 -g -Isrc -c tests/test_info_newwin_fail.c -o obj_test/test_info_newwin_fail.o
gcc -Wall -Wextra -std=c99 -g -Isrc -c src/ui_info.c -o obj_test/ui_info_fail.o
gcc -Wall -Wextra -std=c99 -g -Isrc -c src/dialog.c -o obj_test/dialog_fail.o
gcc -Wall -Wextra -std=c99 -g -Isrc -DUSE_WEAK_MESSAGE -c src/ui_common.c -o obj_test/ui_common_fail.o
gcc obj_test/test_info_newwin_fail.o obj_test/ui_info_fail.o obj_test/dialog_fail.o obj_test/ui_common_fail.o -lncursesw -o test_info_newwin_fail
./test_info_newwin_fail

# build and run UTF-8 print regression test
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc tests/test_utf8_print.c -lncursesw -o test_utf8_print
./test_utf8_print

# build and run confirm quit regression test
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc tests/test_confirm_quit.c \
    obj_test/files.o obj_test/line_buffer.o obj_test/file_manager.o obj_test/globals.o obj_test/stub_enable_color.o -lncursesw -o test_confirm_quit
./test_confirm_quit

# build and run confirm switch regression test
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc \
    tests/test_confirm_switch.c src/editor_actions.c src/file_manager.c src/globals.c \
    obj_test/line_buffer.o -lncursesw -o test_confirm_switch
./test_confirm_switch

# build and run menu overlay clear regression test
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc \
    tests/test_menu_no_clear.c -lncursesw -o test_menu_no_clear
./test_menu_no_clear

# build and run menu switch clear regression test
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc \
    tests/test_menu_switch.c -lncursesw -o test_menu_switch
./test_menu_switch

# build and run version option test
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc \
    tests/test_cli_version.c src/globals.c -lncursesw -o test_cli_version
./test_cli_version

# build and run multi-file main loading test
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc \
    tests/test_main_multifile.c src/globals.c -lncursesw -o test_main_multifile
./test_main_multifile

# build and run select_int valid input test
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc \
    tests/test_select_int_valid.c src/ui_settings.c \
    obj_test/stub_ui_settings_deps.o -lncursesw -o test_select_int_valid
./test_select_int_valid

# build and run select_int invalid input test
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc \
    tests/test_select_int_invalid.c src/ui_settings.c \
    obj_test/stub_ui_settings_deps.o -lncursesw -o test_select_int_invalid
./test_select_int_invalid

# build and run theme option test
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc \
    tests/test_cli_theme.c src/config.c src/globals.c -lncursesw -o test_cli_theme
VENTO_THEME_DIR=./themes ./test_cli_theme

# build and run line option test
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc \
    tests/test_cli_line.c src/globals.c -lncursesw -o test_cli_line
./test_cli_line

# build and run search ignore case test
gcc -Wall -Wextra -std=c99 -g -fsanitize=address -D_POSIX_C_SOURCE=200809L -Isrc \
    tests/test_search_ignore_case.c src/search.c src/line_buffer.c -lncursesw -o test_search_ignore_case
./test_search_ignore_case

# build and run replace ignore case test
gcc -Wall -Wextra -std=c99 -g -fsanitize=address -D_POSIX_C_SOURCE=200809L -Isrc \
    tests/test_replace_ignore_case.c src/search.c src/line_buffer.c -lncursesw -o test_replace_ignore_case
./test_replace_ignore_case

# build and run cursor position restore test
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc \
    tests/test_cursor_restore.c src/editor_actions.c src/file_manager.c src/globals.c \
    obj_test/line_buffer.o -lncursesw -o test_cursor_restore
./test_cursor_restore

# build and run line buffer insert/delete test
gcc -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -Isrc \
    tests/test_linebuffer_insert_delete.c src/line_buffer.c -o test_linebuffer_insert_delete
./test_linebuffer_insert_delete
