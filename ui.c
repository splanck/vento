#include "editor.h"
#include <ncurses.h>
#include <stdlib.h>

void show_help() {
    // Window size and position adjustments
    int win_height = 10;
    int win_width = COLS - 4;
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
    mvwprintw(help_win, 2, 2, "CTRL-S: Save the file");
    mvwprintw(help_win, 3, 2, "CTRL-O: Open a file");
    mvwprintw(help_win, 4, 2, "CTRL-N: New file");
    mvwprintw(help_win, 5, 2, "CTRL-Q: Quit");
    mvwprintw(help_win, 6, 2, "CTRL-H: Show this help");

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
