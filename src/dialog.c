/*
 * Simple pop-up dialog helpers for the editor. These routines
 * create centered ncurses windows and gather short text input
 * from the user.
 */
#include "dialog.h"
#include "ui_common.h"
#include "syntax.h"
#include "config.h"
#include "editor_state.h"
#include <string.h>
#include <ctype.h>

/*
 * Create and display a centered popup dialog window.
 *
 *  height, width - desired size of the dialog window
 *  title        - optional title string printed in bold
 *
 * The window is not automatically repositioned on terminal resize.
 * Returns the WINDOW pointer or NULL on failure.
 */
WINDOW *dialog_open(int height, int width, const char *title) {
    curs_set(0);
    WINDOW *win = create_popup_window(height, width, NULL);
    if (!win)
        return NULL;
    keypad(win, TRUE);
    wbkgd(win, enable_color ? COLOR_PAIR(SYNTAX_BG) : A_NORMAL);
    wrefresh(stdscr);
    box(win, 0, 0);
    if (title && *title) {
        int w;
        int dummy;
        getmaxyx(win, dummy, w);
        (void)dummy; /* avoid unused-variable warning on some compilers */
        wattron(win, A_BOLD);
        mvwprintw(win, 1, (w - (int)strlen(title)) / 2, "%s", title);
        wattroff(win, A_BOLD);
    }
    wrefresh(win);
    return win;
}

/*
 * Close and destroy a dialog window created with dialog_open.
 *
 * win - the WINDOW to destroy; ignored if NULL.
 *
 * This routine does not handle resize events and simply cleans up
 * the window, restoring the cursor.
 */
void dialog_close(WINDOW *win) {
    if (!win)
        return;
    wclear(win);
    wrefresh(win);
    delwin(win);
    wrefresh(stdscr);
    curs_set(1);
}

/*
 * Prompt the user for a single line of text inside the dialog window.
 *
 *  win  - dialog window returned by dialog_open
 *  y,x  - coordinates inside the window where input begins
 *  buf  - buffer receiving the entered text (may contain initial value)
 *  len  - size of buf
 *
 * If the terminal is resized while active, the window is re-centered and
 * redrawn so the prompt remains visible.  Backspace edits the buffer and
 * Escape cancels the prompt.
 *
 * Returns 1 when the user confirms with Enter and 0 if cancelled or on
 * invalid arguments.
 */
int dialog_prompt(WINDOW *win, int y, int x, char *buf, size_t len) {
    if (!win || !buf || len == 0)
        return 0;
    size_t pos = strlen(buf);
    mvwprintw(win, y, x, "%s", buf);
    wmove(win, y, x + (int)pos);
    int ch;
    int cancelled = 0;
    while ((ch = wgetch(win)) != '\n') {
        if (ch == KEY_RESIZE) {
            resizeterm(0, 0);

            int h, w;
            getmaxyx(win, h, w);
            int win_y = (LINES - h) / 2;
            int win_x = (COLS - w) / 2;
            if (win_y < 0)
                win_y = 0;
            if (win_y > LINES - h)
                win_y = LINES - h;
            if (win_x < 0)
                win_x = 0;
            if (win_x > COLS - w)
                win_x = COLS - w;
            mvwin(win, win_y, win_x);
            box(win, 0, 0);
            mvwprintw(win, y, x, "%s", buf);
            wmove(win, y, x + (int)pos);
            wrefresh(stdscr);
            wrefresh(win);
            continue;
        } else if (ch == KEY_BACKSPACE || ch == 127) {
            if (pos > 0) {
                pos--;
                mvwaddch(win, y, x + (int)pos, ' ');
                wmove(win, y, x + (int)pos);
                wrefresh(win);
            }
        } else if (ch == 27) {
            cancelled = 1;
            break;
        } else if (isprint(ch)) {
            if (pos < len - 1) {
                mvwaddch(win, y, x + (int)pos, ch);
                buf[pos++] = (char)ch;
                wmove(win, y, x + (int)pos);
                wrefresh(win);
            }
        }
    }
    buf[pos] = '\0';
    return cancelled ? 0 : 1;
}
