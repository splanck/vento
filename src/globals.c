/* Weak symbols holding default configuration and runtime state. */
/*
 * Default global configuration and runtime flags
 * ----------------------------------------------
 * This file provides weak definitions used by the editor so that other
 * translation units may override them. These globals are updated when
 * configuration files are loaded or command line options are processed.
 */

#include <ncurses.h>
#include "config.h"
#include "file_manager.h"

/* Controls colored output; updated after reading configuration. */
__attribute__((weak)) int enable_color = 1;

/* Controls mouse input handling; changed by configuration. */
__attribute__((weak)) int enable_mouse = 1;

/* Toggle for displaying line numbers; modified via settings. */
__attribute__((weak)) int show_line_numbers = 0;

/* Line number to jump to on file open; cleared after use. */
__attribute__((weak)) int start_line = 0;

/*
 * Global configuration loaded from the user's config file. The structure is
 * populated during config_load and may be saved back to disk. Fields provide
 * default colors, input settings and other editor options.
 */
/* Holds current configuration; modified when config files are loaded. */
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

/*
 * Global file manager tracking all open buffers. Initialized in main() via
 * fm_init and updated as files are opened or closed.
 */
/* Tracks open files; updated whenever files open or close. */
__attribute__((weak)) FileManager file_manager;
