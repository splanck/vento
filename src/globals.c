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

/* Whether color output is enabled. Set by config_load based on the user's
 * preferences and terminal capabilities. */
__attribute__((weak)) int enable_color = 1;

/* Whether mouse support is enabled. Updated after reading the configuration
 * file. */
__attribute__((weak)) int enable_mouse = 1;

/* Display line numbers in the editor. Modified when configuration or UI
 * options change. */
__attribute__((weak)) int show_line_numbers = 0;

/* Initial line to jump to when a file is opened. Reset after the jump is
 * performed. */
__attribute__((weak)) int start_line = 0;

/*
 * Global configuration loaded from the user's config file. The structure is
 * populated during config_load and may be saved back to disk. Fields provide
 * default colors, input settings and other editor options.
 */
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
__attribute__((weak)) FileManager file_manager;
