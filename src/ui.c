/*
 * Utility helpers for generic dialogs used by the editor.
 * These routines implement small find/replace and goto prompts
 * on top of the primitives defined in dialog.c.
 */
#include <ncurses.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include "config.h"
#include "editor.h"
#include "ui.h"
#include "syntax.h"
#include "ui_common.h"
#include "dialog.h"
#include "editor_state.h"

/*
 * Display a generic single-input dialog box.
 *
 *  ctx            - editor context used for color attributes
 *  message        - title displayed at the top of the dialog
 *  output         - buffer receiving the user's text; may be NULL
 *  max_input_len  - size of the output buffer
 *
 * The window is created using dialog_open(), gathers input via
 * dialog_prompt() and is destroyed with dialog_close().  If the user
 * cancels the prompt the output buffer is set to an empty string.
 */
void create_dialog(EditorContext *ctx, const char *message, char *output,
                   int max_input_len) {
    int win_width = (int)strlen(message) + 30;
    WINDOW *dialog_win = dialog_open(7, win_width, message);
    if (!dialog_win) {
        if (output)
            output[0] = '\0';
        return;
    }

    int input_x = 2;
    int input_y = 3;
    if (ctx->enable_color)
        wattron(dialog_win, COLOR_PAIR(SYNTAX_KEYWORD));
    mvwprintw(dialog_win, input_y, input_x, "Input: ");
    if (ctx->enable_color)
        wattroff(dialog_win, COLOR_PAIR(SYNTAX_KEYWORD));
    wrefresh(dialog_win);

    int ok = dialog_prompt(dialog_win, input_y, input_x + 7, output, max_input_len);
    if (!ok)
        output[0] = '\0';

    dialog_close(dialog_win);
}

/*
 * Prompt the user for search text.
 *
 *  ctx            - editor context for color management
 *  output         - buffer receiving the search string
 *  max_input_len  - size of output buffer
 *  preset         - optional initial text to pre-fill the prompt
 *
 * Returns 1 if the user confirms, 0 if cancelled.  Internally this
 * creates a small dialog window with dialog_open(), uses dialog_prompt()
 * for input and closes it via dialog_close().  When cancelled the output
 * buffer is cleared.
 */
int show_find_dialog(EditorContext *ctx, char *output, int max_input_len,
                     const char *preset) {
    int win_width = 40;
    WINDOW *dialog_win = dialog_open(7, win_width, "Find:");
    if (!dialog_win) {
        if (output)
            output[0] = '\0';
        return 0;
    }

    int input_x = 7;
    int input_y = 3;
    if (ctx->enable_color)
        wattron(dialog_win, COLOR_PAIR(SYNTAX_KEYWORD));
    mvwprintw(dialog_win, input_y, input_x, "Input: ");
    if (ctx->enable_color)
        wattroff(dialog_win, COLOR_PAIR(SYNTAX_KEYWORD));
    if (preset && *preset) {
        strncpy(output, preset, max_input_len - 1);
        output[max_input_len - 1] = '\0';
    } else {
        output[0] = '\0';
    }

    int ok = dialog_prompt(dialog_win, input_y, input_x + 7, output, max_input_len);
    if (!ok)
        output[0] = '\0';

    dialog_close(dialog_win);
    return ok;
}

/*
 * Prompt the user for both a search string and its replacement.
 *
 *  ctx              - editor context for color attributes
 *  search           - buffer receiving the text to search for
 *  max_search_len   - size of search buffer
 *  replace          - buffer receiving the replacement text
 *  max_replace_len  - size of replace buffer
 *
 * Returns 1 when both prompts are confirmed, otherwise 0.  The search
 * field is obtained via show_find_dialog() and the replacement text via
 * create_dialog(), both using the helper routines in dialog.c.
 */
int show_replace_dialog(EditorContext *ctx, char *search, int max_search_len,
                        char *replace, int max_replace_len) {
    curs_set(0);
    if (!show_find_dialog(ctx, search, max_search_len, NULL) || search[0] == '\0') {
        curs_set(1);
        return 0;
    }

    create_dialog(ctx, "Replace:", replace, max_replace_len);
    curs_set(1);
    return 1;
}

/*
 * Ask the user for a line number and convert it to an integer.
 *
 *  ctx          - editor context used for the dialog appearance
 *  line_number  - output location for the parsed number on success
 *
 * Returns 1 on success or 0 if the input is cancelled or invalid.
 * The dialog itself is implemented with create_dialog() which in turn
 * uses the dialog.c helpers to manage the popup.
 */
int show_goto_dialog(EditorContext *ctx, int *line_number) {
    char buf[32];

    create_dialog(ctx, "Go To Line:", buf, sizeof(buf));

    if (buf[0] == '\0') {
        return 0;
    }

    char *endptr;
    long val = strtol(buf, &endptr, 10);

    if (*endptr != '\0' || val < 1 || val > INT_MAX) {
        return 0;
    }

    *line_number = (int)val;
    return 1;
}
