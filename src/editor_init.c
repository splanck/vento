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

void disable_ctrl_c_z() {
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
}

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

void initialize(EditorContext *ctx) {
    setlocale(LC_ALL, "");
    initscr();
    config_load(&app_config);
    ctx->config = app_config;
    ctx->enable_color = enable_color;
    ctx->enable_mouse = enable_mouse;
    apply_colors();
    macro_state.length = 0;
    macro_state.recording = false;
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

void cleanup_on_exit(FileManager *fm) {
    if (!fm || !fm->files) {
        freeMenus();
        return;
    }
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

void close_editor() {
    exiting = 1;
}
