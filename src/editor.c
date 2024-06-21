#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include "editor.h"
#include "input.h"
#include "ui.h"
#include "syntax.h"

char *text_buffer[MAX_LINES];
int line_count = 0;
int start_line = 0;

char current_filename[256] = "";

WINDOW *text_win;

void initialize() {
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    init_pair(2, COLOR_BLACK, COLOR_WHITE);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_WHITE, COLOR_BLUE); // White text on blue background

    bkgd(COLOR_PAIR(1));
    refresh();

    // Handle window resizing
    signal(SIGWINCH, handle_resize);

    // Map escape sequences for CTRL-Left and CTRL-Right to custom key constants
    define_key("\033[1;5D", KEY_CTRL_LEFT); // Escape sequence for CTRL-Left
    define_key("\033[1;5C", KEY_CTRL_RIGHT); // Escape sequence for CTRL-Right

    // Map escape sequences for CTRL-Page Up and CTRL-Page Down to custom key constants
    define_key("\033[5;5~", KEY_CTRL_PGUP);  // Escape sequence for CTRL-Page Up
    define_key("\033[6;5~", KEY_CTRL_PGDN);  // Escape sequence for CTRL-Page Down

    // Map escape sequences for CTRL-Up and CTRL-Down to custom key constants
    define_key("\033[1;5A", KEY_CTRL_UP);  // Escape sequence for CTRL-Up
    define_key("\033[1;5B", KEY_CTRL_DOWN);  // Escape sequence for CTRL-Down
}

void run_editor() {
    int ch;
    int cursor_x = 1, cursor_y = 1;

    wmove(text_win, cursor_x, cursor_y);  // Move cursor after "Input: "

    while ((ch = wgetch(text_win)) != 27) { // Exit on ESC key
        switch (ch) {
            case KEY_UP:
                handle_key_up(&cursor_y, &start_line);
                break;
            case KEY_DOWN:
                handle_key_down(&cursor_y, &start_line);
                break;
            case KEY_LEFT:
                handle_key_left(&cursor_x);
                break;
            case KEY_RIGHT:
                handle_key_right(&cursor_x, cursor_y);
                break;
            case KEY_BACKSPACE:
            case 127:
                handle_key_backspace(&cursor_x, &cursor_y, &start_line);
                break;
            case KEY_DC: // Delete key
                handle_key_delete(&cursor_x, cursor_y);
                break;
            case '\n':
                handle_key_enter(&cursor_x, &cursor_y, &start_line);
                break;
            case KEY_PPAGE: // Page Up key
                handle_key_page_up(&cursor_y, &start_line);
                break;
            case KEY_NPAGE: // Page Down key
                handle_key_page_down(&cursor_y, &start_line);
                break;
            case 8: // CTRL-H
                show_help();
                // Restore the text window context
                werase(text_win);
                box(text_win, 0, 0);
                draw_text_buffer(text_win);
                wmove(text_win, cursor_y, cursor_x);
                wrefresh(text_win);
                break;
            case 12: // CTRL-L
                load_file(NULL);
                // Restore the text window context
                werase(text_win);
                box(text_win, 0, 0);
                draw_text_buffer(text_win);
                wmove(text_win, cursor_y, cursor_x);
                wrefresh(text_win);
                break;
            case 15: // CTRL-O
                save_file_as();
                // Restore the text window context
                werase(text_win);
                box(text_win, 0, 0);
                draw_text_buffer(text_win);
                wmove(text_win, cursor_y, cursor_x);
                wrefresh(text_win);
                break;
            case 16: // CTRL-P
                save_file();
                // Restore the text window context
                werase(text_win);
                box(text_win, 0, 0);
                draw_text_buffer(text_win);
                wmove(text_win, cursor_y, cursor_x);
                wrefresh(text_win);
                break;
            case 14: // CTRL-N
                clear_text_buffer();
                cursor_x = 1;
                cursor_y = 1;
                break;
            case KEY_CTRL_LEFT:  // Handle CTRL-Left arrow
                handle_ctrl_key_left(&cursor_x);
                break;
            case KEY_CTRL_RIGHT: // Handle CTRL-Right arrow
                handle_ctrl_key_right(&cursor_x, cursor_y);
                break;
            case KEY_CTRL_PGUP:  // Handle CTRL-Page Up
                handle_ctrl_key_pgup(&cursor_y, &start_line);
                break;
            case KEY_CTRL_PGDN:  // Handle CTRL-Page Down
                handle_ctrl_key_pgdn(&cursor_y, &start_line);
                break;
            case KEY_CTRL_UP:  // Handle CTRL-Up
                handle_ctrl_key_up(&cursor_y);
                break;
            case KEY_CTRL_DOWN:  // Handle CTRL-Down
                handle_ctrl_key_down(&cursor_y);
                break;
            default:
                handle_default_key(ch, &cursor_x, cursor_y);
                break;
        }
        update_status_bar(cursor_y, cursor_x);
        wmove(text_win, cursor_y, cursor_x);  // Restore cursor position
        wrefresh(text_win);
    }

    for (int i = 0; i < MAX_LINES; ++i) {
        free(text_buffer[i]);
    }

    delwin(text_win);
}

void initialize_buffer() {
    for (int i = 0; i < MAX_LINES; ++i) {
        text_buffer[i] = (char *)calloc(COLS - 3, sizeof(char));
    }
    line_count = 1;
    start_line = 0;
}

void draw_text_buffer(WINDOW *win) {
    werase(win);
    box(win, 0, 0);
    int max_lines = LINES - 4;  // Adjust for the status bar
    for (int i = 0; i < max_lines && i + start_line < line_count; ++i) {
        apply_syntax_highlighting(win, text_buffer[i + start_line], i + 1);
    }

    // Calculate scrollbar position and size
    int scrollbar_height = max_lines;
    int scrollbar_start = (start_line * scrollbar_height) / line_count;
    int scrollbar_end = ((start_line + max_lines) * scrollbar_height) / line_count;

    // Draw scrollbar
    for (int i = 0; i < scrollbar_height; ++i) {
        if (i >= scrollbar_start && i < scrollbar_end) {
            mvwprintw(win, i + 1, COLS - 2, "|");
        } else {
            mvwprintw(win, i + 1, COLS - 2, " ");
        }
    }

    wrefresh(win);
}


void handle_resize(int sig) {
    (void)sig;  // Cast to void to suppress unused parameter warning
    endwin();
    refresh();
    clear();

    // Redraw the interface
    werase(text_win);
    wresize(text_win, LINES - 2, COLS);
    mvwin(text_win, 1, 0);
    box(text_win, 0, 0);
    draw_text_buffer(text_win);
    wrefresh(text_win);

    update_status_bar(1, 1);  // Pass some default values for cursor position
}

void clear_text_buffer() {
    for (int i = 0; i < MAX_LINES; ++i) {
        memset(text_buffer[i], 0, COLS - 3);
    }
    line_count = 1;
    start_line = 0;
    werase(text_win);
    box(text_win, 0, 0);
    wrefresh(text_win);
}

void save_file() {
    if (strlen(current_filename) == 0) {
        save_file_as();
    } else {
        FILE *fp = fopen(current_filename, "w");
        if (fp) {
            for (int i = 0; i < line_count; ++i) {
                fprintf(fp, "%s\n", text_buffer[i]);
            }
            fclose(fp);
            mvprintw(LINES - 2, 2, "File saved as %s", current_filename);
        } else {
            mvprintw(LINES - 2, 2, "Error saving file!");
        }
        refresh();
        getch();
        mvprintw(LINES - 2, 2, "                            "); // Clear the line after saving
        refresh();
    }
}

void save_file_as() {
    create_dialog("Save as", current_filename, 256);

    FILE *fp = fopen(current_filename, "w");
    if (fp) {
        for (int i = 0; i < line_count; ++i) {
            fprintf(fp, "%s\n", text_buffer[i]);
        }
        fclose(fp);
        mvprintw(LINES - 2, 2, "File saved as %s", current_filename);
    } else {
        mvprintw(LINES - 2, 2, "Error saving file!");
    }

    refresh();
    getch();
    mvprintw(LINES - 2, 2, "                            "); // Clear the line after saving
    refresh();
}

void load_file(const char *filename) {
    char file_to_load[256];

    // If no filename is provided, prompt the user
    if (filename == NULL) {
        create_dialog("Load file", file_to_load, 256);
        filename = file_to_load;
    }

    initialize_buffer();  // Ensure buffer is initialized

    FILE *fp = fopen(filename, "r");
    if (fp) {
        line_count = 0;
        while (fgets(text_buffer[line_count], COLS - 3, fp) && line_count < MAX_LINES) {
            // Remove newline character if present
            text_buffer[line_count][strcspn(text_buffer[line_count], "\n")] = '\0';
            line_count++;
        }
        fclose(fp);
        mvprintw(LINES - 2, 2, "File loaded: %s", filename);
    } else {
        mvprintw(LINES - 2, 2, "Error loading file!");
    }

    refresh();
    getch();
    mvprintw(LINES - 2, 2, "                            "); // Clear the line after loading
    refresh();
    //werase(text_win);
    text_win = newwin(LINES - 2, COLS, 1, 0);  // Adjusted to ensure proper window size
    keypad(text_win, TRUE);
    meta(text_win, TRUE);  // Enable meta keys

    box(text_win, 0, 0);
    wmove(text_win, 1, 1);

    draw_text_buffer(text_win);
    wrefresh(text_win);
}

void update_status_bar(int cursor_y, int cursor_x) {
    move(LINES - 1, 0);
    clrtoeol();
    mvprintw(LINES - 1, 0, "Lines: %d  Current Line: %d  Column: %d", line_count, cursor_y, cursor_x);
    refresh();
}

void new_file() {
    int cursor_x = 1, cursor_y = 1;

    initialize_buffer(); // Initialize the buffer

    text_win = newwin(LINES - 2, COLS, 1, 0);  // Adjusted to ensure proper window size
    keypad(text_win, TRUE);
    meta(text_win, TRUE);  // Enable meta keys
    box(text_win, 0, 0);
    wmove(text_win, cursor_y, cursor_x);
    wrefresh(text_win);

    run_editor();
}

