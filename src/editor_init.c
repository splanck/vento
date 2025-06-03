/*
 * Editor initialization helpers
 * -----------------------------
 * This file provides routines that bring up and tear down the ncurses
 * environment used by vento.  It loads configuration settings, prepares the
 * menu system and any defined macros, and ensures resources are cleaned up on
 * exit.
 */
#include <ncurses.h>
#include <signal.h>
#include <stdlib.h>
#include <locale.h>
#include "editor.h"
#include "config.h"
#include "ui.h"
#include "menu.h"
#include "file_manager.h"
#include "undo.h"
#include "files.h"
#include "syntax.h"
#include "editor_state.h"
#include "macro.h"

/*
 * Ignore SIGINT and SIGTSTP so Ctrl-C and Ctrl-Z do not suspend ncurses.
 *
 * Side effects:
 *   - Replaces the handlers for SIGINT and SIGTSTP with SIG_IGN.
 */
void disable_ctrl_c_z() {
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
}

/*
 * Apply the configured color theme to the main window and all open file
 * buffers.
 *
 * Side effects:
 *   - Updates the background attribute of stdscr and every FileState window
 *     based on the global enable_color flag.
 */
void apply_colors() {
    bkgd(enable_color ? COLOR_PAIR(SYNTAX_BG) : A_NORMAL);
    for (int i = 0; i < file_manager.count; ++i) {
        FileState *fs = file_manager.files[i];
        if (fs && fs->text_win) {
            wbkgd(fs->text_win,
                  enable_color ? COLOR_PAIR(SYNTAX_BG) : A_NORMAL);
        }
    }
}

/*
 * Initialize the editor runtime and screen.
 *
 * ctx - EditorContext that receives configuration and state information.
 *
 * Side effects:
 *   - Initializes ncurses, loads the configuration file and updates global
 *     options such as enable_color and enable_mouse.
 *   - Creates a default macro if none exist.
 *   - Registers signal handlers and key definitions, and prepares menus.
 */
void initialize(EditorContext *ctx) {
    setlocale(LC_ALL, "");
    initscr();
    config_load(&app_config);
    ctx->config = app_config;
    ctx->enable_color = enable_color;
    ctx->enable_mouse = enable_mouse;
    apply_colors();
    if (macro_count() == 0) {
        current_macro = macro_create("default");
        if (current_macro) {
            current_macro->length = 0;
            current_macro->recording = false;
        }
    }
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    meta(stdscr, TRUE);
    if (ctx->enable_mouse)
        mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    else
        mousemask(0, NULL);
    timeout(10);
    bkgd(ctx->enable_color ? COLOR_PAIR(1) : A_NORMAL);
    refresh();
    struct sigaction sa;
    sa.sa_handler = on_sigwinch;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
#ifdef SIGWINCH
    /* Some platforms may not support SIGWINCH */
    sigaction(SIGWINCH, &sa, NULL);
#endif
    disable_ctrl_c_z();
    define_key("\033[1;5D", KEY_CTRL_LEFT);
    define_key("\033[1;5C", KEY_CTRL_RIGHT);
    define_key("\033[5;5~", KEY_CTRL_PGUP);
    define_key("\033[6;5~", KEY_CTRL_PGDN);
    define_key("\033[1;5A", KEY_CTRL_UP);
    define_key("\033[1;5B", KEY_CTRL_DOWN);
    define_key("\024", KEY_CTRL_T);
    initialize_key_mappings();
    initializeMenus(ctx);
    update_status_bar(ctx, ctx->active_file);
}

/*
 * Release resources before shutting down.
 *
 * fm - FileManager containing all open files.
 *
 * Side effects:
 *   - Saves macros and clears syntax highlighting data.
 *   - Frees every FileState along with the menu structures.
 *   - Resets the FileManager fields to an empty state.
 */
void cleanup_on_exit(FileManager *fm) {
    if (!fm || !fm->files) {
        freeMenus();
        return;
    }
    macros_save(&app_config);
    macros_free_all();
    syntax_cleanup();
    for (int i = 0; i < fm->count; ++i) {
        FileState *fs = fm->files[i];
        if (!fs) continue;
        free_file_state(fs);
    }
    freeMenus();
    free(fm->files);
    fm->files = NULL;
    fm->count = 0;
    fm->active_index = -1;
}

/*
 * Signal the main loop to terminate.
 *
 * Side effects:
 *   - Sets the global `exiting` flag which causes run_editor() to stop.
 */
void close_editor() {
    exiting = 1;
}
