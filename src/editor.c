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
#include "menu.h"
#include "config.h"

char *strdup(const char *s);  // Explicitly declare strdup
int exiting = 0;

char *text_buffer[MAX_LINES];
int line_count = 0;
int start_line = 0;
int runeditor = 0;

char current_filename[256] = "";

WINDOW *text_win;

// Global clipboard
char *clipboard;
bool selection_mode = false;
int sel_start_x = 0, sel_start_y = 0;
int sel_end_x = 0, sel_end_y = 0;

// Undo and redo stacks
Node *undo_stack = NULL;
Node *redo_stack = NULL;

// Helper functions for undo and redo stacks
void push(Node **stack, Change change) {
    Node *new_node = (Node *)malloc(sizeof(Node));
    new_node->change = change;
    new_node->next = *stack;
    *stack = new_node;
}

Change pop(Node **stack) {
    if (*stack == NULL) {
        Change empty_change = {0, NULL, NULL};
        return empty_change;
    }
    Node *top = *stack;
    Change change = top->change;
    *stack = top->next;
    free(top);
    return change;
}

int is_empty(Node *stack) {
    return stack == NULL;
}

void undo() {
    if (undo_stack == NULL) return;
    Change change = pop(&undo_stack);

    if (change.old_text) {
        // If old_text is not NULL, it means we are restoring a modified line
        // Shift lines down to make space for the restored line
        for (int i = line_count; i > change.line; --i) {
            strcpy(text_buffer[i], text_buffer[i - 1]);
        }
        line_count++;

        // Restore the old state
        strcpy(text_buffer[change.line], change.old_text);
        push(&redo_stack, (Change){change.line, strdup(change.old_text), NULL});
        free(change.old_text);
    } else {
        // If old_text is NULL, it means we are undoing an inserted line
        // Shift lines up to remove the inserted line
        for (int i = change.line; i < line_count - 1; ++i) {
            strcpy(text_buffer[i], text_buffer[i + 1]);
        }
        text_buffer[line_count - 1][0] = '\0';
        line_count--;
        push(&redo_stack, change);
    }

    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(text_win);
    wrefresh(text_win);
}

void redo() {
    if (redo_stack == NULL) return;
    Change change = pop(&redo_stack);
    if (change.new_text) {
        // Restore the new state
        push(&undo_stack, (Change){change.line, strdup(text_buffer[change.line]), strdup(change.new_text)});
        strcpy(text_buffer[change.line], change.new_text);
        free(change.new_text);
    }
    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(text_win);
}

void start_selection_mode(int cursor_x, int cursor_y) {
    selection_mode = true;
    sel_start_x = cursor_x;
    sel_start_y = cursor_y;
    sel_end_x = cursor_x;
    sel_end_y = cursor_y;
}

void end_selection_mode() {
    selection_mode = false;
    copy_selection();
}

void copy_selection() {
    int start_y = sel_start_y < sel_end_y ? sel_start_y : sel_end_y;
    int end_y = sel_start_y > sel_end_y ? sel_start_y : sel_end_y;
    int start_x = sel_start_x;
    int end_x = sel_end_x;

    clipboard[0] = '\0';  // Clear clipboard
    for (int y = start_y; y <= end_y; y++) {
        if (y == start_y) {
            strncat(clipboard, &text_buffer[y - 1 + start_line][start_x - 1], end_x - start_x + 1);
        } else {
            strcat(clipboard, "\n");
            strcat(clipboard, text_buffer[y - 1 + start_line]);
        }
    }
}

void paste_clipboard(int *cursor_x, int *cursor_y) {
    char *line = strtok(clipboard, "\n");
    while (line) {
        int len = strlen(line);
        memmove(&text_buffer[*cursor_y - 1 + start_line][*cursor_x - 1 + len], &text_buffer[*cursor_y - 1 + start_line][*cursor_x - 1], strlen(&text_buffer[*cursor_y - 1 + start_line][*cursor_x - 1]) + 1);
        memcpy(&text_buffer[*cursor_y - 1 + start_line][*cursor_x - 1], line, len);
        line = strtok(NULL, "\n");
        (*cursor_y)++;
        *cursor_x = 1;
    }
}

void handle_selection_mode(int ch, int *cursor_x, int *cursor_y) {
    switch (ch) {
        case KEY_UP:
            if (*cursor_y > 1) (*cursor_y)--;
            sel_end_y = *cursor_y;
            break;
        case KEY_DOWN:
            if (*cursor_y < LINES - 4) (*cursor_y)++;
            sel_end_y = *cursor_y;
            break;
        case KEY_LEFT:
            if (*cursor_x > 1) (*cursor_x)--;
            sel_end_x = *cursor_x;
            break;
        case KEY_RIGHT:
            if (*cursor_x < COLS - 6) (*cursor_x)++;
            sel_end_x = *cursor_x;
            break;
        case 10: // CTRL-J to end selection mode
            end_selection_mode();
            break;
        default:
            break;
    }
}

void delete_current_line(int *cursor_y, int *start_line) {
    if (line_count == 0) {
        return;
    }

    int line_to_delete = *cursor_y - 1 + *start_line;

    // Push the current state to the undo stack
    push(&undo_stack, (Change){line_to_delete, strdup(text_buffer[line_to_delete]), NULL});

    // Shift lines up to delete the current line
    for (int i = line_to_delete; i < line_count - 1; ++i) {
        strcpy(text_buffer[i], text_buffer[i + 1]);
    }

    // Clear the last line
    memset(text_buffer[line_count - 1], 0, COLS - 3);
    line_count--;

    // Move cursor to the next line if possible
    if (*cursor_y < LINES - 4 && *cursor_y <= line_count) {
        // Move to the next line
    } else if (*start_line + *cursor_y > line_count) {
        // Move up if at the end of the document
        if (*cursor_y > 1) {
            (*cursor_y)--;
        } else if (*start_line > 0) {
            (*start_line)--;
        }
    }

    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(text_win);
}

void insert_new_line(int *cursor_x, int *cursor_y, int *start_line) {
    if (line_count < MAX_LINES - 1) {
        // Move lines below the cursor down by one
        for (int i = line_count; i > *cursor_y + *start_line - 1; --i) {
            strcpy(text_buffer[i], text_buffer[i - 1]);
        }
        line_count++;

        // Insert a new empty line at the current cursor position
        text_buffer[*cursor_y + *start_line - 1][0] = '\0';

        // Record the change for undo
        Change change;
        change.line = *cursor_y + *start_line - 1;
        change.old_text = NULL;
        change.new_text = strdup("");

        push(&undo_stack, change);

        // Move cursor to the new line
        *cursor_x = 1;

        // Adjust cursor_y and start_line
        if (*cursor_y == LINES - 4 && *start_line + LINES - 4 < line_count) {
            (*start_line)++;
        } else {
            (*cursor_y)++;
        }

        // Clear and redraw the text window
        redraw(cursor_x, cursor_y);

        // Move the cursor up one line
        if (*cursor_y > 1) {
            (*cursor_y)--;
        } else if (*start_line > 0) {
            (*start_line)--;
        }
    }
}

void disable_ctrl_c_z() {
    // Ignore SIGINT (CTRL-C)
    signal(SIGINT, SIG_IGN);
    // Ignore SIGTSTP (CTRL-Z)
    signal(SIGTSTP, SIG_IGN);
}

void initialize() {
    for (int i = 0; i < MAX_LINES; ++i) {
        text_buffer[i] = (char *)malloc(sizeof(char));
    }

    initscr();
    read_config_file();

    if (enable_color) {
        start_color();
    }

    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    meta(stdscr, TRUE);  // Enable 8-bit control characters
    keypad(text_win, TRUE);
    meta(text_win, TRUE);  // Enable 8-bit control characters for text_win

    read_config_file();

    if (has_colors() && can_change_color()) {
        start_color();
        init_pair(1, COLOR_WHITE, COLOR_BLUE);     // Background color
        init_pair(2, COLOR_CYAN, COLOR_BLACK);     // Keywords
        init_pair(3, COLOR_GREEN, COLOR_BLACK);    // Comments
        init_pair(4, COLOR_YELLOW, COLOR_BLACK);   // Strings
        init_pair(5, COLOR_MAGENTA, COLOR_BLACK);  // Types
        init_pair(6, COLOR_BLUE, COLOR_BLACK);      // Symbols (braces, parentheses)
    }

    bkgd(COLOR_PAIR(1));
    refresh();

    // Handle window resizing
    signal(SIGWINCH, handle_resize);
    
    disable_ctrl_c_z();

    // Map escape sequences for CTRL-Left and CTRL-Right to custom key constants
    define_key("\033[1;5D", KEY_CTRL_LEFT); // Escape sequence for CTRL-Left
    define_key("\033[1;5C", KEY_CTRL_RIGHT); // Escape sequence for CTRL-Right

    // Map escape sequences for CTRL-Page Up and CTRL-Page Down to custom key constants
    define_key("\033[5;5~", KEY_CTRL_PGUP);  // Escape sequence for CTRL-Page Up
    define_key("\033[6;5~", KEY_CTRL_PGDN);  // Escape sequence for CTRL-Page Down

    // Map escape sequences for CTRL-Up and CTRL-Down to custom key constants
    define_key("\033[1;5A", KEY_CTRL_UP);  // Escape sequence for CTRL-Up
    define_key("\033[1;5B", KEY_CTRL_DOWN);  // Escape sequence for CTRL-Down

    // Map CTRL-T to custom key constant
    define_key("\024", KEY_CTRL_T); // \024 is the octal for CTRL-T

    initializeMenus();
    update_status_bar(0, 0);  
}

void run_editor() {
    if (runeditor == 0)
        runeditor = 1;
    else
        return;

    int ch;
    int cursor_x = 1, cursor_y = 1;
    int currentMenu = 0;
    int currentItem = 0;

    wmove(text_win, cursor_x, cursor_y);  // Move cursor after "Input: "

    while ((ch = wgetch(text_win)) && exiting == 0) { // Exit on ESC key
        if (ch == ERR) {
            continue; // Handle any errors or no input case
        }

        if (exiting == 1) {
            break;
        }

        //mvprintw(LINES - 1, 0, "Pressed key: %d", ch); // Add this line for debugging
        update_status_bar(cursor_x, cursor_y);        
        refresh();
        
        if (selection_mode) {
            handle_selection_mode(ch, &cursor_x, &cursor_y);
        } else if (ch == KEY_CTRL_T) { // CTRL-T
            refresh();
            handleMenuNavigation(menus, menuCount, &currentMenu, &currentItem);
            // Redraw the editor screen after closing the menu
            redraw(&cursor_x, &cursor_y);
        } else {
            handle_regular_mode(ch, &cursor_x, &cursor_y);
        }

        if (exiting == 1)
            break;

        update_status_bar(cursor_y + start_line, cursor_x); // Update status bar with absolute line number
        wmove(text_win, cursor_y, cursor_x);  // Restore cursor position
        wrefresh(text_win);
    }

    delwin(text_win);
}

void cleanup_on_exit() {
    for (int i = 0; i < MAX_LINES; ++i) {
        if (text_buffer[i] != NULL) {  // Check for NULL pointer
            free(text_buffer[i]);
            text_buffer[i] = NULL;  // Set to NULL to avoid double free
        }
    }
}

void close_editor() {
    exiting = 1;
}

void initialize_buffer() {
    for (int i = 0; i < MAX_LINES; ++i) {
        text_buffer[i] = (char *)calloc(COLS - 3, sizeof(char));
        if (text_buffer[i] == NULL) {
            // Handle allocation failure
            fprintf(stderr, "Memory allocation failed for text_buffer[%d]\n", i);
            exit(1);
        }
    }
    line_count = 1;
    start_line = 0;
}

void draw_text_buffer(WINDOW *win) {
    if (start_line < 0)
        start_line = 0;
        
    werase(win);
    box(win, 0, 0);
    int max_lines = LINES - 4;  // Adjust for the status bar
    for (int i = 0; i < max_lines && i + start_line < line_count; ++i) {
        apply_syntax_highlighting(win, text_buffer[i + start_line], i + 1);
    }

    // Calculate scrollbar position and size
    int scrollbar_height = max_lines;
    int scrollbar_start = 0;
    int scrollbar_end = 0;

    if (line_count > 0) {
        scrollbar_start = (start_line * scrollbar_height) / line_count;
        scrollbar_end = ((start_line + max_lines) * scrollbar_height) / line_count;
    }

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

void handle_regular_mode(int ch, int *cursor_x, int *cursor_y) {
    switch (ch) {
        case KEY_UP:
            handle_key_up(cursor_y, &start_line);
            break;
        case KEY_DOWN:
            handle_key_down(cursor_y, &start_line);
            break;
        case KEY_LEFT:
            handle_key_left(cursor_x);
            break;
        case KEY_RIGHT:
            handle_key_right(cursor_x, *cursor_y);
            break;
        case KEY_BACKSPACE:
        case 127:
            handle_key_backspace(cursor_x, cursor_y, &start_line);
            break;
        case KEY_DC: // Delete key
            handle_key_delete(cursor_x, *cursor_y);
            break;
        case '\n':
            handle_key_enter(cursor_x, cursor_y, &start_line);
            break;
        case KEY_PPAGE: // Page Up key
            handle_key_page_up(cursor_y, &start_line);
            break;
        case KEY_NPAGE: // Page Down key
            handle_key_page_down(cursor_y, &start_line);
            redraw(cursor_x, cursor_y);
            break;
        case 8: // CTRL-H
            show_help();
            redraw(cursor_x, cursor_y);
            break;
        case 1: // CTRL-A
            show_about();
            redraw(cursor_x, cursor_y);
            break;
        case 4: // CTRL-D to delete current line
            delete_current_line(cursor_y, &start_line);
            break;
        case 9: // CTRL-I to insert a new line
            insert_new_line(cursor_x, cursor_y, &start_line);
            break;
        case 6: // CTRL-F to move forward to the next word
            move_forward_to_next_word(cursor_x, cursor_y);
            break;
        case 2: // CTRL-B to move backward to the previous word
            move_backward_to_previous_word(cursor_x, cursor_y);
            break;
        case 12: // CTRL-L
            load_file(NULL);
            redraw(cursor_x, cursor_y);
            break;
        case 15: // CTRL-O
            save_file_as();
            redraw(cursor_x, cursor_y);
            break;
        case 16: // CTRL-P
            save_file();
            redraw(cursor_x, cursor_y);
            break;
        case 13: // CTRL-; to start/stop selection mode
            if (selection_mode) {
                end_selection_mode();
            } else {
                start_selection_mode(*cursor_x, *cursor_y);
            }
            break;
        case 11: // CTRL-K to paste clipboard
            paste_clipboard(cursor_x, cursor_y);
            break;
        case 14: // CTRL-N
            clear_text_buffer();
            *cursor_x = 1;
            *cursor_y = 1;
            break;
        case 18: // CTRL-R
            redo();
            break;
        case 21: // CTRL-U
            undo();
            break;
        case 24: // CTRL-X to quit
            exiting = 1;
            break;
        case KEY_CTRL_LEFT:  // Handle CTRL-Left arrow
            handle_ctrl_key_left(cursor_x);
            break;
        case KEY_CTRL_RIGHT: // Handle CTRL-Right arrow
            handle_ctrl_key_right(cursor_x, *cursor_y);
            break;
        case KEY_CTRL_PGUP:  // Handle CTRL-Page Up
            handle_ctrl_key_pgup(cursor_y, &start_line);
            break;
        case KEY_CTRL_PGDN:  // Handle CTRL-Page Down
            handle_ctrl_key_pgdn(cursor_y, &start_line);
            break;
        case KEY_CTRL_UP:  // Handle CTRL-Up
            handle_ctrl_key_up(cursor_y);
            break;
        case KEY_CTRL_DOWN:  // Handle CTRL-Down
            handle_ctrl_key_down(cursor_y);
            break;
        default:
            handle_default_key(ch, cursor_x, *cursor_y);
            break;
    }
}

void redraw(int *cursor_x, int *cursor_y) {
    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(text_win);
    wmove(text_win, *cursor_y, *cursor_x);
    wrefresh(text_win);
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
    start_line = 1;
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

void set_syntax_mode(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (ext) {
        if (strcmp(ext, ".c") == 0 || strcmp(ext, ".h") == 0) {
            current_syntax_mode = C_SYNTAX;
        } else if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) {
            current_syntax_mode = HTML_SYNTAX;
        } else if (strcmp(ext, ".py") == 0) {
            current_syntax_mode = PYTHON_SYNTAX;
        } else {
            current_syntax_mode = NO_SYNTAX;
        }
    } else {
        current_syntax_mode = NO_SYNTAX;
    }
}

void load_file(const char *filename) {
    char file_to_load[256];

    // If no filename is provided, prompt the user
    if (filename == NULL) {
        create_dialog("Load file", file_to_load, 256);
        filename = file_to_load;
    }

    set_syntax_mode(filename);
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

        strcpy(current_filename, filename);
    } else {
        mvprintw(LINES - 2, 2, "Error loading file!");
    }

    refresh();
    
    // Wait for a brief moment to display the message, without requiring Enter
    timeout(100); // Wait for 500 milliseconds
    getch();      // Consume any key press if available
    timeout(-1);  // Reset to blocking mode

    mvprintw(LINES - 2, 2, "                            "); // Clear the line after loading
    refresh();
    text_win = newwin(LINES - 2, COLS, 1, 0);  // Adjusted to ensure proper window size
    keypad(text_win, TRUE);
    meta(text_win, TRUE);  // Enable meta keys

    box(text_win, 0, 0);
    wmove(text_win, 1, 1);

    draw_text_buffer(text_win);
    wrefresh(text_win);
}

void update_status_bar(int cursor_y, int cursor_x) {
    // Display the filename centered on line 2
    move(0, 0);
    int filename_length = strlen(current_filename);
    int center_position = (COLS - filename_length) / 2;
    mvprintw(1, center_position, "%s", current_filename);

    // Display the status bar at the bottom
    move(LINES - 1, 0);
    clrtoeol();
    int actual_line_number = cursor_y + start_line; // Calculate actual line number
    mvprintw(LINES - 1, 0, "Lines: %d  Current Line: %d  Column: %d", line_count, actual_line_number, cursor_x);

    // Display "CTRL-H - Help" at the bottom right
    mvprintw(LINES - 1, COLS - 15, "CTRL-H - Help");

    refresh();
}

void new_file() {
    int cursor_x = 1, cursor_y = 1;
    strcpy(current_filename, "");

    initialize_buffer(); // Initialize the buffer

    text_win = newwin(LINES - 2, COLS, 1, 0);  // Adjusted to ensure proper window size
    keypad(text_win, TRUE);
    meta(text_win, TRUE);  // Enable meta keys
    box(text_win, 0, 0);
    wmove(text_win, cursor_y, cursor_x);
    wrefresh(text_win);
}
