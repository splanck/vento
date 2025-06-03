#include <ncurses.h>
#include "config.h"
#include "file_manager.h"

// global application state
__attribute__((weak)) int enable_color = 1;
__attribute__((weak)) int enable_mouse = 1;
__attribute__((weak)) int show_line_numbers = 0;
__attribute__((weak)) int start_line = 0;

__attribute__((weak)) AppConfig app_config = {
    .background_color = "BLACK",
    .text_color = "WHITE",
    .keyword_color = "CYAN",
    .comment_color = "GREEN",
    .string_color = "YELLOW",
    .type_color = "MAGENTA",
    .symbol_color = "RED",
    .search_color = "YELLOW",
    .theme = "",
    .enable_color = 1,
    .enable_mouse = 1,
    .show_line_numbers = 0,
    .show_startup_warning = 1,
    .search_ignore_case = 0,
    .tab_width = 4,
    .macros_file = "",
    .macro_record_key = KEY_F(2),
    .macro_play_key = KEY_F(4)
};

__attribute__((weak)) FileManager file_manager;
