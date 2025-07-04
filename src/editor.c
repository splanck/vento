#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED
#endif
/*
 * vento's editor implementation.
 *
 * This file drives the main editing interface.  It sets up the table of
 * keyboard shortcuts used by the editor and contains the core event loop
 * responsible for processing input.  The routines here draw text buffers,
 * coordinate input handling and screen updates using ncurses.
 */
#include <ncurses.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>

#if defined(NCURSES_WIDECHAR) || defined(_XOPEN_SOURCE_EXTENDED)
#define USE_WIDE_INPUT 1
#else
#define USE_WIDE_INPUT 0
#endif

#include "editor.h"
#include "input.h"
#include "ui.h"
#include "syntax.h"
#include "menu.h"
#include "config.h"
#include "files.h"
#include "undo.h"
#include "clipboard.h"
#include "file_ops.h"
#include "search.h"
#include "file_manager.h"
#include "editor_state.h"
#include "macro.h"

__attribute__((weak)) void copy_selection_keyboard(FileState *fs) {
    (void)fs;
}

__attribute__((weak)) void cut_selection(FileState *fs) {
    (void)fs;
}

void allocation_failed(const char *msg) {
    endwin();
    if (msg)
        fprintf(stderr, "%s\n", msg);
    else
        fprintf(stderr, "Memory allocation failed\n");
    exit(EXIT_FAILURE);
}

FileState *active_file = NULL;
/* Each FileState holds its own LineBuffer in the `buffer` field */
int runeditor = 0;  // Flag to control the main loop of the editor
WINDOW *text_win;  // Pointer to the ncurses window for displaying the text
__attribute__((weak)) int show_line_numbers;

// Undo and redo stacks are stored per file in FileState

int exiting = 0;

char search_text[256] = "";

volatile sig_atomic_t resize_pending = 0;
static EditorContext *input_ctx = NULL;
Macro *current_macro = NULL;

void clamp_scroll_x(FileState *fs) {
    int offset = get_line_number_offset(fs);
    int visible_width = COLS - 2 - offset;
    if (visible_width < 1)
        visible_width = 1;
    if (fs->cursor_x - 1 < fs->scroll_x)
        fs->scroll_x = fs->cursor_x - 1;
    if (fs->scroll_x < 0)
        fs->scroll_x = 0;
    if (fs->cursor_x - 1 >= fs->scroll_x + visible_width)
        fs->scroll_x = fs->cursor_x - visible_width;
    if (fs->scroll_x < 0)
        fs->scroll_x = 0;
}

void on_sigwinch(int sig) {
    (void)sig;
    resize_pending = 1;
}

__attribute__((weak)) int get_line_number_offset(FileState *fs) {
    if (!show_line_numbers || !fs)
        return 0;
    int lines = fs->buffer.count > 0 ? fs->buffer.count : 1;
    int width = 1;
    while (lines >= 10) { width++; lines /= 10; }
    return width + 1;
}

int key_help = 8;  // Key code for the help command (CTRL-H)
int key_f1_help = KEY_F(1);  // F1 also opens help
int key_about = 1;  // Key code for the about command
int key_delete_line = 4;  // Key code for the delete line command
int key_insert_line = KEY_F(5);  // Key code for the insert line command
int key_macro_record = KEY_F(2);  // Key code for starting/stopping macro recording
int key_macro_play   = KEY_F(4);  // Key code for playing back a recorded macro
int key_move_forward = 23;  // Key code for the move forward command (CTRL-w)
int key_move_backward = 2;  // Key code for the move backward command
int key_load_file = 12;  // Key code for the load file command
int key_save_as = 15;  // Key code for the save as command
int key_save_file = 19;  // Key code for the save file command (CTRL-S)
int key_close_file = 17;  // Key code for closing the current file (CTRL-Q)
int key_selection_mode = 13;  // Key code for entering selection mode
int key_paste_clipboard = 22;  // Key code for pasting from clipboard (CTRL-V)
int key_copy_selection = 3;   // Key code for copying the selection (CTRL-C)
int key_cut_selection = 24;   // Key code for cutting the selection (CTRL-X)
int key_clear_buffer = 14;  // Key code for clearing the text buffer
int key_redo = 25;  // Key code for the redo command (CTRL-Y)
int key_undo = 26;  // Key code for the undo command (CTRL-Z)
int key_find = 6;  // Key code for finding next word
int key_find_next = KEY_F(3);  // Key code for find next occurrence
int key_next_file = KEY_F(6);  // Key code for switching to the next file
int key_prev_file = KEY_F(7);  // Key code for switching to the previous file
int key_replace = 18;  // Key code for replacing text (CTRL-R)
int key_goto_line = 7;  // Key code for go to line (CTRL-G)
int key_menu_open = KEY_F(10);  // Key code for opening the menu (F10)

static void handle_key_up_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    handle_key_up(input_ctx, fs);
}

static void handle_key_down_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    handle_key_down(input_ctx, fs);
}

static void handle_key_left_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    handle_key_left(input_ctx, fs);
}

static void handle_key_right_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    handle_key_right(input_ctx, fs);
}

static void handle_key_backspace_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    handle_key_backspace(input_ctx, fs);
}

static void handle_key_delete_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    handle_key_delete(input_ctx, fs);
}

static void handle_key_enter_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    handle_key_enter(input_ctx, fs);
}

static void handle_key_home_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    handle_key_home(input_ctx, fs);
}

static void handle_key_end_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    handle_key_end(input_ctx, fs);
}

static void handle_key_page_up_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    handle_key_page_up(input_ctx, fs);
}

static void handle_key_page_down_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    handle_key_page_down(input_ctx, fs);
    redraw();
}

static void handle_ctrl_left_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    handle_ctrl_key_left(input_ctx, fs);
}

static void handle_ctrl_right_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    handle_ctrl_key_right(input_ctx, fs);
}

static void handle_ctrl_pgup_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    handle_ctrl_key_pgup(input_ctx, fs);
}

static void handle_ctrl_pgdn_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    handle_ctrl_key_pgdn(input_ctx, fs);
}

static void handle_ctrl_up_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    handle_ctrl_key_up(input_ctx, fs);
}

static void handle_ctrl_down_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    handle_ctrl_key_down(input_ctx, fs);
}

static void handle_help_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    (void)cx;
    (void)cy;
    show_help(input_ctx);
    redraw();
}

static void handle_about_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    (void)cx;
    (void)cy;
    show_about(input_ctx);
    redraw();
}

static void handle_find_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    find(input_ctx, fs, 1);
    redraw();
}

static void handle_find_next_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    find(input_ctx, fs, 0);
    redraw();
}

static void handle_replace_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    replace(input_ctx, fs);
    redraw();
}

static void handle_goto_line_wrapper(struct FileState *fs, int *cx, int *cy) {
    int line;
    if (show_goto_dialog(input_ctx, &line)) {
        go_to_line(input_ctx, fs, line);
        *cx = fs->cursor_x;
        *cy = fs->cursor_y;
    }
}

static void handle_delete_line_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    delete_current_line(input_ctx, fs);
    *cy = fs->cursor_y;
}

static void handle_insert_line_wrapper(struct FileState *fs, int *cx, int *cy) {
    insert_new_line(input_ctx, fs);
    *cx = fs->cursor_x;
    *cy = fs->cursor_y;
}

static void handle_move_forward_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    move_forward_to_next_word(input_ctx, fs);
}

static void handle_move_backward_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    move_backward_to_previous_word(input_ctx, fs);
}

static void handle_load_file_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    load_file(input_ctx, fs, NULL);
    redraw();
}

static void handle_save_as_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    save_file_as(input_ctx, fs);
    redraw();
}

static void handle_save_file_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    save_file(input_ctx, fs);
    redraw();
}

static void handle_close_file_wrapper(struct FileState *fs, int *cx, int *cy) {
    close_current_file(input_ctx, fs, cx, cy);
}

static void handle_selection_mode_wrapper(struct FileState *fs, int *cx, int *cy) {
    if (fs->selection_mode) {
        end_selection_mode(fs);
    } else {
        start_selection_mode(fs, *cx, *cy);
    }
}

static void handle_paste_clipboard_wrapper(struct FileState *fs, int *cx, int *cy) {
    paste_clipboard(fs, cx, cy);
}

static void handle_copy_selection_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    copy_selection_keyboard(fs);
}

static void handle_cut_selection_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    cut_selection(fs);
}

static void handle_next_file_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    (void)cx;
    (void)cy;
    next_file(input_ctx);
}

static void handle_prev_file_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    (void)cx;
    (void)cy;
    prev_file(input_ctx);
}

static void handle_clear_buffer_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    clear_text_buffer();
    *cx = 1;
    *cy = 1;
}

static void handle_macro_record_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    (void)cx;
    (void)cy;
    if (current_macro && current_macro->recording)
        macro_stop(current_macro);
    else
        macro_start(current_macro);
    if (input_ctx)
        update_status_bar(input_ctx, input_ctx->active_file);
}

static void handle_macro_play_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    if (current_macro)
        macro_play(current_macro, input_ctx, fs);
    if (input_ctx)
        update_status_bar(input_ctx, input_ctx->active_file);
}



#define MAX_KEY_MAPPINGS 64
static KeyMapping key_mappings[MAX_KEY_MAPPINGS];
static int key_mapping_count = 0;

/*
 * Populate the array of KeyMapping structures with key codes and the
 * functions that handle them.  Each entry pairs a keyboard shortcut with
 * its wrapper so the main loop can dispatch events quickly.  The editor
 * initialization code calls this once at startup.
 */
void initialize_key_mappings(void) {
    key_mapping_count = 0;

    key_mappings[key_mapping_count++] = (KeyMapping){KEY_UP, handle_key_up_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){KEY_DOWN, handle_key_down_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){KEY_LEFT, handle_key_left_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){KEY_RIGHT, handle_key_right_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){KEY_HOME, handle_key_home_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){KEY_END, handle_key_end_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){KEY_BACKSPACE, handle_key_backspace_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){127, handle_key_backspace_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){KEY_DC, handle_key_delete_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){'\n', handle_key_enter_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){KEY_PPAGE, handle_key_page_up_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){KEY_NPAGE, handle_key_page_down_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){KEY_CTRL_LEFT, handle_ctrl_left_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){KEY_CTRL_RIGHT, handle_ctrl_right_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){KEY_CTRL_PGUP, handle_ctrl_pgup_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){KEY_CTRL_PGDN, handle_ctrl_pgdn_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){KEY_CTRL_UP, handle_ctrl_up_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){KEY_CTRL_DOWN, handle_ctrl_down_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_help, handle_help_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_f1_help, handle_help_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_about, handle_about_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_find, handle_find_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_find_next, handle_find_next_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_replace, handle_replace_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_goto_line, handle_goto_line_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_delete_line, handle_delete_line_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_insert_line, handle_insert_line_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_move_forward, handle_move_forward_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_move_backward, handle_move_backward_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_load_file, handle_load_file_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_save_as, handle_save_as_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_save_file, handle_save_file_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_close_file, handle_close_file_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_selection_mode, handle_selection_mode_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_paste_clipboard, handle_paste_clipboard_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_copy_selection, handle_copy_selection_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_cut_selection, handle_cut_selection_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_clear_buffer, handle_clear_buffer_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_redo, handle_redo_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_undo, handle_undo_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_next_file, handle_next_file_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_prev_file, handle_prev_file_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_macro_record, handle_macro_record_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_macro_play, handle_macro_play_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){KEY_CTRL_T, NULL}; /* placeholder for menu key, handled elsewhere */
    key_mappings[key_mapping_count++] = (KeyMapping){key_menu_open, NULL}; /* placeholder for F10 menu key */

    key_mappings[key_mapping_count++] = (KeyMapping){0, NULL}; /* terminator */
}








/*
 * Main event loop driving the editor UI.  It continually polls for keyboard
 * and mouse events, dispatching them to the appropriate handlers.  Key presses
 * may be recorded into a macro when recording is active.  After each action,
 * the status bar and text window are refreshed until the user exits.
 */
void run_editor(EditorContext *ctx) {
    input_ctx = ctx;
    if (runeditor == 0)
        runeditor = 1;
    else
        return;

    wint_t ch;
    int currentMenu = 0;
    int currentItem = 0;
    MEVENT event; // Mouse event structure

    wmove(ctx->text_win, ctx->active_file->cursor_y,
          ctx->active_file->cursor_x + get_line_number_offset(ctx->active_file));

    drawBar();
    update_status_bar(ctx, ctx->active_file);
    doupdate();

    while (exiting == 0) {
        int rc;
#if USE_WIDE_INPUT
        rc = wget_wch(ctx->text_win, &ch);  // poll for keyboard input
#else
        int ch_tmp = wgetch(ctx->text_win); // poll for keyboard input
        rc = (ch_tmp == ERR) ? ERR : OK;
        ch = ch_tmp;
#endif
        if (resize_pending || ch == KEY_RESIZE) {
            perform_resize();
            resize_pending = 0;
            if (ch == KEY_RESIZE)
                continue;              /* skip further processing */
        }

        if (rc == ERR) {
            continue; // No input available
        }

        /* Record the key if a macro is currently being recorded */
        if (current_macro && current_macro->recording &&
            ch != (wint_t)KEY_CTRL_BACKTICK && ch != (wint_t)KEY_CTRL_Q) {
            macro_record_key(current_macro, ch);
        }

        if (exiting == 1) {
            break;
        }

        // Update menus and status bar before handling the key
        drawBar();
        update_status_bar(ctx, ctx->active_file);
        doupdate();
        
        /* Dispatch the input to the appropriate handler */
        if (ctx->active_file->selection_mode) {
            handle_selection_mode(ctx->active_file, ch, &ctx->active_file->cursor_x, &ctx->active_file->cursor_y);
        } else if (ch == (wint_t)KEY_CTRL_T || ch == (wint_t)key_menu_open) { // CTRL-T or F10
            doupdate();
            handleMenuNavigation(menus, menuCount, &currentMenu, &currentItem);
            // Redraw the editor screen after closing the menu
            redraw();
        } else if (ch == KEY_MOUSE && ctx->enable_mouse) {
            if (getmouse(&event) == OK) {
                if (menu_click_open(event.x, event.y)) {
                    flushinp();
                    redraw();
                } else {
                    handle_mouse_event(ctx, ctx->active_file, &event);
                }
            }
        } else if (ch == (wint_t)key_macro_record) {
            handle_macro_record_wrapper(ctx->active_file, &ctx->active_file->cursor_x, &ctx->active_file->cursor_y);
        } else {
            Macro *play = NULL;
            for (int i = 0; i < macro_count(); ++i) {
                Macro *m = macro_at(i);
                if (m && m->play_key && m->play_key == (int)ch) {
                    play = m;
                    break;
                }
            }
            if (play) {
                macro_play(play, ctx, ctx->active_file);
            } else if (ch == (wint_t)key_macro_play) {
                handle_macro_play_wrapper(ctx->active_file, &ctx->active_file->cursor_x, &ctx->active_file->cursor_y);
            } else {
                handle_regular_mode(ctx, ctx->active_file, ch);
            }
        }

        if (exiting == 1)
            break;

        /* Refresh status bar and screen after processing the event */
        update_status_bar(ctx, ctx->active_file);
        wmove(ctx->text_win, ctx->active_file->cursor_y,
              ctx->active_file->cursor_x + get_line_number_offset(ctx->active_file));  // Restore cursor position
        wnoutrefresh(ctx->text_win);
        doupdate();
    }

}



/*
 * Allocate and reset the active file's line buffer.
 * Called when a file is opened or the buffer is cleared so that editing
 * starts with a clean slate.
 */
void initialize_buffer() {
    if (!active_file)
        return;

    /* Allocate memory for each line in the text buffer */
    for (int i = 0; i < active_file->buffer.capacity; ++i) {
        if (active_file->buffer.lines[i] != NULL) {
            free(active_file->buffer.lines[i]);
            active_file->buffer.lines[i] = NULL;
        }
        active_file->buffer.lines[i] =
            (char *)calloc(active_file->line_capacity, sizeof(char));
        if (active_file->buffer.lines[i] == NULL) {
            allocation_failed("calloc failed in initialize_buffer");
        }
    }

    /* Set the initial line count and start line */
    active_file->buffer.count = 1;
    active_file->start_line = 0;
}

/*
 * Render the visible portion of a FileState to the given ncurses window.
 * Handles line numbers, syntax highlighting and scrollbar drawing.
 */
void draw_text_buffer(FileState *fs, WINDOW *win) {
    if (fs->start_line < 0)
        fs->start_line = 0;

    sync_multiline_comment(fs, fs->start_line);

    werase(win);
    box(win, 0, 0);
    int max_lines = LINES - 4;  // Adjust for the status bar

    int num_width = 0;
    int offset = 0;
    WINDOW *content = win;
    if (show_line_numbers) {
        int lines = fs->buffer.count > 0 ? fs->buffer.count : 1;
        num_width = 1;
        while (lines >= 10) { num_width++; lines /= 10; }
        offset = num_width + 1; /* numbers plus space */
        content = derwin(win, getmaxy(win), getmaxx(win) - offset, 0, offset);
        if (!content) content = win;
    }

    // Ensure enough lines are loaded for display
    ensure_line_loaded(fs, fs->start_line + max_lines);

    int visible_width = COLS - 2 - offset;
    if (visible_width < 1)
        visible_width = 1;

    // Iterate over each line to be displayed on the window
    for (int i = 0; i < max_lines && i + fs->start_line < fs->buffer.count; ++i) {
        int line_idx = i + fs->start_line;
        if (show_line_numbers) {
            mvwprintw(win, i + 1, 1, "%*d ", num_width, line_idx + 1);
        }
        const char *line = fs->buffer.lines[line_idx];
        const char *start = line + fs->scroll_x;
        int use_w = visible_width;
        char *temp = malloc(use_w + 1);
        if (!temp) {
            allocation_failed("malloc failed");
            return;
        }
        strncpy(temp, start, use_w);
        temp[use_w] = '\0';
        // Apply syntax highlighting to the current line of text
        apply_syntax_highlighting(fs, content, temp, i + 1);
        free(temp);

        // Highlight current search match if it falls on this line
        if (fs->match_start_y == line_idx && fs->match_start_x >= 0) {
            int start_x = fs->match_start_x - fs->scroll_x + offset;
            int len = fs->match_end_x - fs->match_start_x + 1;
            if (start_x < COLS - 2 && len > 0) {
                if (start_x + len > COLS - 2)
                    len = COLS - 2 - start_x;
                mvwchgat(win, i + 1, start_x + 1, len,
                         COLOR_PAIR(SYNTAX_SEARCH) | A_BOLD, 0, NULL);
            }
        }
    }

    // Calculate scrollbar position and size
    int scrollbar_height = max_lines;
    int scrollbar_start = 0;
    int scrollbar_end = 0;

    if (fs->buffer.count > 0) {
        scrollbar_start = (fs->start_line * scrollbar_height) / fs->buffer.count;
        scrollbar_end = ((fs->start_line + max_lines) * scrollbar_height) / fs->buffer.count;
    }

    // Draw scrollbar
    for (int i = 0; i < scrollbar_height; ++i) {
        if (i >= scrollbar_start && i < scrollbar_end) {
            mvwprintw(win, i + 1, COLS - 1, "|");
        } else {
            mvwprintw(win, i + 1, COLS - 1, " ");
        }
    }

    // Draw scrollbar at the top if the cursor is at the very top of the document
    if (fs->start_line == 0 && scrollbar_start == 0 && scrollbar_end > 0) {
        mvwprintw(win, 1, COLS - 1, "|");
    }

    if (content != win)
        delwin(content);
    wrefresh(win);
}

/**
 * This function handles the regular mode of the editor, where the user can navigate and edit the text.
 * It takes a character input 'ch' and the current cursor position 'cursor_x' and 'cursor_y' as parameters.
 * Based on the input, it performs various actions such as moving the cursor, deleting characters, inserting new lines, etc.
 * 
 * @param ch The character input from the user.
 * @param cursor_x The current x-coordinate of the cursor.
 * @param cursor_y The current y-coordinate of the cursor.
 * @return None
 */
void handle_regular_mode(EditorContext *ctx, FileState *fs, wint_t ch) {
    fs->match_start_x = fs->match_end_x = -1;
    fs->match_start_y = fs->match_end_y = -1;
    for (int i = 0; i < key_mapping_count; ++i) {
        if (key_mappings[i].key == 0 && key_mappings[i].handler == NULL) {
            break;
        }
        if (key_mappings[i].key == (int)ch) {
            if (key_mappings[i].handler) {
                key_mappings[i].handler(fs, &fs->cursor_x, &fs->cursor_y);
            }
            return;
        }
    }

    handle_default_key(ctx, fs, ch);
}

/**
 * Redraws the editor screen after a resize event.
 * 
 * This function clears the text window, redraws the text buffer, and moves the cursor to its previous position.
 * 
 * @param cursor_x A pointer to the current x-coordinate of the cursor.
 * @param cursor_y A pointer to the current y-coordinate of the cursor.
 * @return None
 */
void redraw() {
    werase(text_win); // Clear the text window
    box(text_win, 0, 0); // Redraw the border of the text window
    draw_text_buffer(active_file, text_win); // Redraw the text buffer
    wmove(text_win, active_file->cursor_y,
          active_file->cursor_x + get_line_number_offset(active_file)); // Move the cursor to its previous position
    wnoutrefresh(text_win); // Queue refresh for the text window
}

/**
 * Handles the resize signal.
 * 
 * This function is called when the terminal window is resized. It clears the screen, redraws the interface,
 * and updates the status bar.
 *
 * @return None
 */
void perform_resize(void) {

    if (!active_file || !active_file->text_win)
        return;

    endwin(); // End the curses mode
    resizeterm(0, 0); // update ncurses internal size
    clear(); // Clear the screen

    int new_capacity = COLS - 3;
    if (new_capacity < 1)
        new_capacity = 1;

    /* Resize all open file windows and buffers */
    for (int i = 0; i < file_manager.count; ++i) {
        FileState *fs = file_manager.files[i];
        if (!fs || !fs->text_win) {
            continue;
        }
        wresize(fs->text_win, LINES - 2, COLS);
        mvwin(fs->text_win, 1, 0);

        if (ensure_col_capacity(fs, new_capacity) < 0)
            allocation_failed("ensure_col_capacity failed");
        clamp_scroll_x(fs);
    }

    /* Use the resized window of the active file */
    text_win = active_file->text_win;
    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(active_file, text_win);

    if (active_file->cursor_x > COLS - 1)
        active_file->cursor_x = COLS - 1;
    if (active_file->cursor_x < 1)
        active_file->cursor_x = 1;

    clamp_scroll_x(active_file);

    if (active_file->cursor_y > LINES - BOTTOM_MARGIN)
        active_file->cursor_y = LINES - BOTTOM_MARGIN;
    if (active_file->cursor_y < 1)
        active_file->cursor_y = 1;

    update_status_bar(input_ctx, active_file);
    wmove(text_win, active_file->cursor_y,
          active_file->cursor_x + get_line_number_offset(active_file));
    wnoutrefresh(text_win);

    /* Redraw the menu bar after all windows have been updated */
    drawBar();
    doupdate();
    flushinp();
}

/**
 * Clears the text buffer.
 * 
 * This function clears the text buffer by setting all elements to 0 and resets the line count and start line variables.
 * It also clears the text window, redraws the border, and refreshes the window.
 * 
 * @return None
 */
void clear_text_buffer() {
    if (!active_file)
        return;

    // Set all elements of the text buffer to 0
    for (int i = 0; i < active_file->buffer.capacity; ++i) {
        memset(active_file->buffer.lines[i], 0, active_file->line_capacity);
    }

    // Reset line count and start line variables
    active_file->buffer.count = 1;
    if (active_file)
        active_file->start_line = 0;

    if (active_file->undo_stack) {
        free_stack(active_file->undo_stack);
        active_file->undo_stack = NULL;
    }

    if (active_file->redo_stack) {
        free_stack(active_file->redo_stack);
        active_file->redo_stack = NULL;
    }
    
    // Clear the text window
    werase(text_win);
    
    // Redraw the border of the text window
    box(text_win, 0, 0);
    
    // Refresh the text window
    wrefresh(text_win);
}



