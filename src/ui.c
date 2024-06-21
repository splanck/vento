#include "editor.h"
#include <ncurses.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

void show_help() {
    // Window size and position adjustments
    int win_height = 12;
    int win_width = COLS - 40;
    int win_y = (LINES - win_height) / 2;
    int win_x = (COLS - win_width) / 2;

    // Create a temporary window for the help dialog
    WINDOW *help_win = newwin(win_height, win_width, win_y, win_x);
    keypad(help_win, TRUE);  // Enable keyboard input for the help dialog

    // Set color pair for the help window
    wbkgd(help_win, COLOR_PAIR(1));  // Use the defined color pair
    wrefresh(stdscr);  // Refresh the main screen before drawing the dialog

    // Draw the help dialog borders
    box(help_win, 0, 0);

    // Print the help information
    mvwprintw(help_win, 1, 2, "Help:");
    mvwprintw(help_win, 3, 2, "CTRL-S: Save the file");
    mvwprintw(help_win, 4, 2, "CTRL-O: Open a file");
    mvwprintw(help_win, 5, 2, "CTRL-N: New file");
    mvwprintw(help_win, 6, 2, "CTRL-U: Undo");
    mvwprintw(help_win, 7, 2, "CTRL-R: Redo");
    mvwprintw(help_win, 8, 2, "CTRL-H: Show this help");

    // Wait for any keypress to close the dialog
    mvwprintw(help_win, win_height - 2, 2, "(Press any key to close)");
    wrefresh(help_win);
    wgetch(help_win);

    // Clean up the help window
    wclear(help_win);
    wrefresh(help_win);
    delwin(help_win);
    wrefresh(stdscr);  // Refresh the main screen after closing the dialog
}

void create_dialog(const char *message, char *output, int max_input_len) {
    // Window size and position adjustments
    int win_y = LINES / 3;
    int win_x = (COLS - strlen(message) - 8) / 2;
    int win_width = strlen(message) + 30;
    int win_height = 5;

    // Create a temporary window for the dialog
    WINDOW *dialog_win = newwin(win_height, win_width, win_y, win_x);
    keypad(dialog_win, TRUE);  // Enable keyboard input for the dialog
    wrefresh(stdscr);  // Refresh the main screen before drawing the dialog

    // Draw the dialog borders
    box(dialog_win, 0, 0);

    // Print the message at the center
    mvwprintw(dialog_win, 1, (win_width - strlen(message)) / 2, message);

    // Input field and cursor position
    int input_x = 2;  // Adjust as needed for message positioning
    int input_y = 3;
    mvwprintw(dialog_win, input_y, input_x, "Input: ");
    wmove(dialog_win, input_y, input_x + 7);  // Move cursor after "Input: "

    // Get user input with maximum length check
    int ch, input_len = 0;
    while ((ch = wgetch(dialog_win)) != '\n' && input_len < max_input_len - 1) {
        if (isprint(ch)) {
            mvwprintw(dialog_win, input_y, input_x + 7 + input_len, "%c", ch);
            output[input_len] = ch;
            input_len++;
            wrefresh(dialog_win);
        } else if (ch == KEY_BACKSPACE || ch == 127) {
            if (input_len > 0) {
                input_len--;
                mvwprintw(dialog_win, input_y, input_x + 7 + input_len, " ");
                wmove(dialog_win, input_y, input_x + 7 + input_len);
                wrefresh(dialog_win);
            }
        }
    }
    output[input_len] = '\0';  // Add null terminator to the input string

    // Clean up the dialog window
    wclear(dialog_win);
    wrefresh(dialog_win);
    delwin(dialog_win);
    wrefresh(stdscr);  // Refresh the main screen after closing the dialog
}
