#include "editor.h"
#include <ncurses.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "config.h"

void show_help() {
    // Window size and position adjustments
    int win_height = 15;
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

    // Print the help information in two columns
    mvwprintw(help_win, 1, 2, "Help:");
    mvwprintw(help_win, 3, 2, "CTRL-H: Show this help");
    mvwprintw(help_win, 4, 2, "CTRL-A: About");
    mvwprintw(help_win, 5, 2, "CTRL-L: Load a new file");
    mvwprintw(help_win, 6, 2, "CTRL-O: Save as");
    mvwprintw(help_win, 7, 2, "CTRL-P: Save");
    mvwprintw(help_win, 8, 2, "CTRL-J: Start/Stop selection mode");
    mvwprintw(help_win, 9, 2, "CTRL-K: Paste from clipboard");
    mvwprintw(help_win, 10, 2, "CTRL-N: New file");
    mvwprintw(help_win, 11, 2, "CTRL-R: Redo");
    mvwprintw(help_win, 12, 2, "CTRL-U: Undo");

    mvwprintw(help_win, 3, win_width / 2, "CTRL-X: Quit");
    mvwprintw(help_win, 4, win_width / 2, "CTRL-F: Move forward to next word");
    mvwprintw(help_win, 5, win_width / 2, "CTRL-B: Move backward to previous word");
    mvwprintw(help_win, 6, win_width / 2, "CTRL-D: Delete current line");
    mvwprintw(help_win, 7, win_width / 2, "Arrow Keys: Navigate text");
    mvwprintw(help_win, 8, win_width / 2, "Page Up/Down: Scroll document");
    mvwprintw(help_win, 9, win_width / 2, "CTRL-Left: Move to line start");
    mvwprintw(help_win, 10, win_width / 2, "CTRL-Right: Move to line end");
    mvwprintw(help_win, 11, win_width / 2, "CTRL-PgUp: Move to top of doc");
    mvwprintw(help_win, 12, win_width / 2, "CTRL-PgDn: Move to end of doc");

    // Wait for any keypress to close the dialog
    mvwprintw(help_win, win_height - 1, (win_width - strlen("(Press any key to close)")) / 2, "(Press any key to close)");
    wrefresh(help_win);
    wgetch(help_win);

    // Clean up the help window
    wclear(help_win);
    wrefresh(help_win);
    delwin(help_win);
    wrefresh(stdscr);  // Refresh the main screen after closing the dialog
}

void show_about() {
    // Window size and position adjustments
    int win_height = 10;
    int win_width = COLS - 20;
    int win_y = (LINES - win_height) / 2;
    int win_x = (COLS - win_width) / 2;

    // Create a temporary window for the about dialog
    WINDOW *about_win = newwin(win_height, win_width, win_y, win_x);
    keypad(about_win, TRUE);  // Enable keyboard input for the about dialog

    // Set color pair for the about window
    wbkgd(about_win, COLOR_PAIR(1));  // Use the defined color pair
    wrefresh(stdscr);  // Refresh the main screen before drawing the dialog

    // Draw the about dialog borders
    box(about_win, 0, 0);

    // Print the about information
    mvwprintw(about_win, 1, 2, "Vento Text Editor");
    mvwprintw(about_win, 2, 2, "Version: %s", VERSION);
    mvwprintw(about_win, 3, 2, "License: GPL v3");
    mvwprintw(about_win, 4, 2, "Vento is open-source software licensed under the GPL v3.");

    // Wait for any keypress to close the dialog
    mvwprintw(about_win, win_height - 2, 2, "(Press any key to close)");
    wrefresh(about_win);
    wgetch(about_win);

    // Clean up the about window
    wclear(about_win);
    wrefresh(about_win);
    delwin(about_win);
    wrefresh(stdscr);  // Refresh the main screen after closing the dialog
}

void create_dialog(const char *message, char *output, int max_input_len) {
    // Define colors
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLUE); // Background color for dialog
    init_pair(2, COLOR_YELLOW, COLOR_BLACK); // Input text color

    // Window size and position adjustments
    int win_y = LINES / 3;
    int win_x = (COLS - strlen(message) - 30) / 2;
    int win_width = strlen(message) + 30;
    int win_height = 7;

    // Create a temporary window for the dialog
    WINDOW *dialog_win = newwin(win_height, win_width, win_y, win_x);
    keypad(dialog_win, TRUE);  // Enable keyboard input for the dialog
    wbkgd(dialog_win, COLOR_PAIR(1)); // Set background color
    wrefresh(stdscr);  // Refresh the main screen before drawing the dialog

    // Draw the dialog borders
    box(dialog_win, 0, 0);

    // Print the message at the center
    wattron(dialog_win, A_BOLD); // Bold the message
    mvwprintw(dialog_win, 1, (win_width - strlen(message)) / 2, message);
    wattroff(dialog_win, A_BOLD);

    // Input field and cursor position
    int input_x = 2; // Adjust as needed for message positioning
    int input_y = 3;
    wattron(dialog_win, COLOR_PAIR(2)); // Set input text color
    mvwprintw(dialog_win, input_y, input_x, "Input: ");
    wattroff(dialog_win, COLOR_PAIR(2));
    wmove(dialog_win, input_y, input_x + 7);  // Move cursor after "Input: "

    // Get user input with maximum length check
    int ch, input_len = 0;
    while ((ch = wgetch(dialog_win)) != '\n' && input_len < max_input_len - 1) {
        if (isprint(ch)) {
            mvwprintw(dialog_win, input_y, input_x + 7 + input_len, "%c", ch);
            output[input_len] = ch;
            input_len++;
            wmove(dialog_win, input_y, input_x + 7 + input_len);
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
