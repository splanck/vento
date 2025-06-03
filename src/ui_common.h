/*
 * Common window and dialog helper functions.
 */
#ifndef UI_COMMON_H
#define UI_COMMON_H

#include <ncurses.h>
#include <stddef.h>

/**
 * Create a new window centered in the terminal or within an optional parent.
 *
 * The requested size is clamped to fit on screen. When a parent window is
 * provided, the new window is positioned relative to that parent's
 * coordinates. Ownership of the returned WINDOW is transferred to the caller.
 */
WINDOW *create_centered_window(int height, int width, WINDOW *parent);

/**
 * Wrapper around create_centered_window for pop-up dialogs.
 *
 * If allocation fails an error message is shown. The caller becomes
 * responsible for destroying the returned window.
 */
WINDOW *create_popup_window(int height, int width, WINDOW *parent);

/**
 * Display a short message centered on screen.
 *
 * The window size is derived from the message length. The temporary window is
 * created and destroyed within this call. The pressed key is returned or ERR
 * on failure.
 */
int show_message(const char *msg);

/**
 * Present a scrollable list of strings to the user.
 *
 * When a parent window is given, the list window fills most of the parent and
 * is centered within it. Otherwise it uses roughly 70% of the terminal size.
 * The popup window is owned and destroyed by the function. The return value is
 * the index of the selected item or -1 when cancelled.
 */
int show_scrollable_window(const char **options, int count, WINDOW *parent,
                           int width);
void str_to_upper(char *dst, const char *src, size_t dst_size);

#endif // UI_COMMON_H
