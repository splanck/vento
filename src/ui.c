#include "editor.h"
#include <ncurses.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include "config.h"
#include "ui.h"

// Ensure the correct prototypes are available
char *strdup(const char *s);

/**
 * Displays the help dialog with a list of available commands and their corresponding keybindings.
 */
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

    // Print the title
    mvwprintw(help_win, 1, 2, "Help:");

    // Print the command and keybinding for each command
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

    // Print the second column of commands and keybindings
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

/**
 * Displays the about dialog with information about the Vento Text Editor.
 */
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

    // Print the title
    mvwprintw(about_win, 1, 2, "Vento Text Editor");

    // Print the version
    mvwprintw(about_win, 2, 2, "Version: %s", VERSION);

    // Print the license
    mvwprintw(about_win, 3, 2, "License: GPL v3");

    // Print the description
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

/**
 * Creates a dialog window with a message and an input field for the user to enter text.
 * The entered text is stored in the 'output' parameter.
 * The maximum length of the input is specified by 'max_input_len'.
 *
 * @param message The message to display in the dialog window.
 * @param output  The buffer to store the user's input.
 * @param max_input_len The maximum length of the user's input.
 */
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

/**
 * Displays a warning dialog to inform the user that the software is experimental and not intended for production use.
 * The dialog will be dismissed when any key is pressed.
 */
void show_warning_dialog() {
    int win_height = 7;
    int win_width = COLS - 20;
    int win_y = (LINES - win_height) / 2;
    int win_x = (COLS - win_width) / 2;

    // Create a temporary window for the warning dialog
    WINDOW *warning_win = newwin(win_height, win_width, win_y, win_x);
    keypad(warning_win, TRUE);  // Enable keyboard input for the warning dialog

    // Set color pair for the warning window
    wbkgd(warning_win, COLOR_PAIR(1));
    wrefresh(stdscr);  // Refresh the main screen before drawing the dialog

    // Draw the warning dialog borders
    box(warning_win, 0, 0);

    // Print the warning message centered
    char *message1 = "Warning: This is experimental software.";
    char *message2 = "It is under development and not intended for production use.";
    char *message3 = "(Press any key to dismiss)";

    mvwprintw(warning_win, 2, (win_width - strlen(message1)) / 2, "%s", message1);
    mvwprintw(warning_win, 3, (win_width - strlen(message2)) / 2, "%s", message2);
    mvwprintw(warning_win, 5, (win_width - strlen(message3)) / 2, "%s", message3);

    wrefresh(warning_win);

    // Wait for any keypress to close the dialog
    wgetch(warning_win);

    // Refresh the entire screen
    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(text_win);
    wrefresh(text_win);
    update_status_bar(1, 1);
    wrefresh(stdscr);  // Refresh the main screen after closing the dialog
}

void get_dir_contents(const char *dir_path, char ***choices, int *n_choices) {
    DIR *dir;
    struct dirent *entry;
    int count = 0;

    dir = opendir(dir_path);
    if (!dir) {
        *choices = NULL;
        *n_choices = 0;
        return;
    }

    // Count entries
    while ((entry = readdir(dir)) != NULL) {
        count++;
    }
    rewinddir(dir);

    *choices = (char **)malloc(count * sizeof(char *));
    int i = 0;
    while ((entry = readdir(dir)) != NULL) {
        (*choices)[i] = strdup(entry->d_name);
        i++;
    }
    closedir(dir);

    *n_choices = count;
}

void free_dir_contents(char **choices, int n_choices) {
    for (int i = 0; i < n_choices; ++i) {
        free(choices[i]);
    }
    free(choices);
}

int show_select_file(char *selected_path, int max_path_len) {
    int highlight = 0;
    // int choice = 0;
    int ch;
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));

    while (1) {
        clear();
        mvprintw(0, 0, "Current Directory: %s", cwd);

        char **choices = NULL;
        int n_choices = 0;
        get_dir_contents(cwd, &choices, &n_choices);

        for (int i = 0; i < n_choices; ++i) {
            if (i == highlight)
                attron(A_REVERSE);
            mvprintw(i + 1, 0, "%s", choices[i]);
            attroff(A_REVERSE);
        }

        ch = getch();
        switch (ch) {
            case KEY_UP:
                if (highlight > 0)
                    --highlight;
                break;
            case KEY_DOWN:
                if (highlight < n_choices - 1)
                    ++highlight;
                break;
            case '\n':
                if (n_choices > 0) {
                    struct stat statbuf;
                    char next_path[2048]; // Increase the buffer size to avoid truncation
                    snprintf(next_path, sizeof(next_path), "%s/%s", cwd, choices[highlight]);
                    if (stat(next_path, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
                        strncpy(cwd, next_path, sizeof(cwd));
                        cwd[sizeof(cwd) - 1] = '\0'; // Ensure null termination
                        highlight = 0;
                    } else {
                        strncpy(selected_path, next_path, max_path_len);
                        selected_path[max_path_len - 1] = '\0'; // Ensure null termination
                        free_dir_contents(choices, n_choices);
                        return 1;
                    }
                }
                break;
            case 27: // ESC key
                free_dir_contents(choices, n_choices);
                return 0;
        }

        free_dir_contents(choices, n_choices);
    }

    // Refresh the entire screen
    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(text_win);
    wrefresh(text_win);
    update_status_bar(1, 1);
    wrefresh(stdscr);  // Refresh the main screen after closing the dialog
}

/**
 * Shows a find dialog window and accepts a string from the user.
 * The entered string is stored in the 'output' parameter.
 * The maximum length of the input is specified by 'max_input_len'.
 *
 * @param output The buffer to store the user's input.
 * @param max_input_len The maximum length of the user's input.
 */
void show_find_dialog(char *output, int max_input_len) {
    // Define colors
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLUE); // Background color for dialog
    init_pair(2, COLOR_YELLOW, COLOR_BLACK); // Input text color

    // Window size and position adjustments
    int win_y = LINES / 3;
    int win_x = (COLS - 40) / 2;
    int win_width = 40;
    int win_height = 7;

    // Create a temporary window for the dialog
    WINDOW *dialog_win = newwin(win_height, win_width, win_y, win_x);
    keypad(dialog_win, TRUE);  // Enable keyboard input for the dialog
    wbkgd(dialog_win, COLOR_PAIR(1)); // Set background color
    wrefresh(stdscr);  // Refresh the main screen before drawing the dialog

    // Draw the dialog borders
    box(dialog_win, 0, 0);

    // Print the message at the center
    char *message = "Find: ";
    wattron(dialog_win, A_BOLD); // Bold the message
    mvwprintw(dialog_win, 1, (win_width - strlen(message)) / 2, message);
    wattroff(dialog_win, A_BOLD);

    // Input field and cursor position
    int input_x = 7; // Adjust as needed for message positioning
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
