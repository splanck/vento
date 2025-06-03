/*
 * Mouse input handling
 * --------------------
 * This module updates text selections when dragging with the mouse and
 * supports scrolling through wheel button presses.  BUTTON1_DRAGGED events
 * extend the active selection, while BUTTON4_PRESSED and BUTTON5_PRESSED
 * trigger vertical scrolling just like the arrow keys.  Not all systems
 * define those wheel button constants, so their usage is wrapped in
 * conditional compilation checks.
 */
#include "editor.h"
#include <ncurses.h>
#include "input.h"
#include "clipboard.h"
#include "editor_state.h"

#ifndef BUTTON1_DRAGGED
#define BUTTON1_DRAGGED (BUTTON1_PRESSED | REPORT_MOUSE_POSITION)
#endif

/*
 * Extend the current selection to the given coordinates.
 *
 * ctx - editor context (unused)
 * fs  - file state to modify
 * x,y - new end position for the selection (1-based)
 *
 * Writes to fs->sel_end_x and fs->sel_end_y and redraws the text window
 * so the highlighted region stays up to date.
 */
void update_selection_mouse(EditorContext *ctx, FileState *fs, int x, int y) {
    (void)ctx;
    fs->sel_end_x = x;
    fs->sel_end_y = y;

    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(fs, text_win);
    wmove(text_win, fs->cursor_y,
          fs->cursor_x + get_line_number_offset(fs));
    wrefresh(text_win);
}

/*
 * Process a single ncurses mouse event.
 *
 * ctx - active editor context
 * fs  - file state updated by the event
 * ev  - mouse event information from getmouse()
 *
 * Updates the cursor position and selection markers.  Wheel events scroll
 * through the buffer using handle_key_up/down.  BUTTON4_PRESSED and
 * BUTTON5_PRESSED checks are wrapped in #ifdefs for portability on systems
 * that may lack these constants.
 */
void handle_mouse_event(EditorContext *ctx, FileState *fs, MEVENT *ev) {
    fs->match_start_x = fs->match_end_x = -1;
    fs->match_start_y = fs->match_end_y = -1;
    int mx = ev->x;
    int my = ev->y - 1; // account for window border
    int offset = get_line_number_offset(fs);

    if (mx >= offset && mx < COLS - 2 && my >= 0 && my < LINES - BOTTOM_MARGIN) {
        fs->cursor_x = mx - offset + 1;
        fs->cursor_y = my + 1;
    }

    if (ev->bstate & BUTTON1_PRESSED) {
        start_selection_mode(fs, fs->cursor_x, fs->cursor_y);
    }

    if (ev->bstate & BUTTON1_RELEASED) {
        end_selection_mode(fs);
    }

    if (ev->bstate & BUTTON1_DRAGGED) {
        update_selection_mouse(ctx, fs, fs->cursor_x, fs->cursor_y);
    }

#ifdef BUTTON4_PRESSED
    /* Some systems may not define BUTTON4_PRESSED */
    if (ev->bstate & BUTTON4_PRESSED) {
        handle_key_up(ctx, fs);
    }
#endif

#ifdef BUTTON5_PRESSED
    /* Some systems may not define BUTTON5_PRESSED */
    if (ev->bstate & BUTTON5_PRESSED) {
        handle_key_down(ctx, fs);
    }
#endif
}
