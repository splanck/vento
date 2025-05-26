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
#include "files.h"
#include "undo.h"
#include "clipboard.h"

int cursor_x = 1, cursor_y = 1;
char current_filename[256] = "";  // Name of the current file being edited
char *text_buffer[MAX_LINES];  // Array of strings to store the lines of text
int line_count = 0;  // Number of lines in the text buffer
int start_line = 0;  // Index of the first visible line in the text window
int runeditor = 0;  // Flag to control the main loop of the editor
WINDOW *text_win;  // Pointer to the ncurses window for displaying the text

// Undo and redo stacks
Node *undo_stack = NULL;
Node *redo_stack = NULL;

char *strdup(const char *s);  // Explicitly declare strdup
int exiting = 0;

char search_text[256] = "";

int key_help = 8;  // Key code for the help command
int key_about = 1;  // Key code for the about command
int key_delete_line = 4;  // Key code for the delete line command
int key_insert_line = KEY_F(5);  // Key code for the insert line command
int key_move_forward = 23;  // Key code for the move forward command (CTRL-w)
int key_move_backward = 2;  // Key code for the move backward command
int key_load_file = 12;  // Key code for the load file command
int key_save_as = 15;  // Key code for the save as command
int key_save_file = 16;  // Key code for the save file command
int key_selection_mode = 13;  // Key code for entering selection mode
int key_paste_clipboard = 11;  // Key code for pasting from clipboard
int key_clear_buffer = 14;  // Key code for clearing the text buffer
int key_redo = 18;  // Key code for the redo command
int key_undo = 21;  // Key code for the undo command
int key_quit = 24;  // Key code for quitting the editor
int key_find = 6;  // Key code for finding next word

static void handle_key_up_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    (void)cx;
    handle_key_up(cy, &start_line);
}

static void handle_key_down_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    (void)cx;
    handle_key_down(cy, &start_line);
}

static void handle_key_left_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    (void)cy;
    handle_key_left(cx);
}

static void handle_key_right_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    handle_key_right(cx, *cy);
}

static void handle_key_backspace_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    handle_key_backspace(cx, cy, &start_line);
}

static void handle_key_delete_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    handle_key_delete(cx, *cy);
}

static void handle_key_enter_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    handle_key_enter(cx, cy, &start_line);
}

static void handle_key_page_up_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    (void)cx;
    handle_key_page_up(cy, &start_line);
}

static void handle_key_page_down_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    handle_key_page_down(cy, &start_line);
    redraw(cx, cy);
}

static void handle_ctrl_left_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    (void)cy;
    handle_ctrl_key_left(cx);
}

static void handle_ctrl_right_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    handle_ctrl_key_right(cx, *cy);
}

static void handle_ctrl_pgup_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    (void)cx;
    handle_ctrl_key_pgup(cy, &start_line);
}

static void handle_ctrl_pgdn_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    (void)cx;
    handle_ctrl_key_pgdn(cy, &start_line);
}

static void handle_ctrl_up_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    (void)cx;
    handle_ctrl_key_up(cy);
}

static void handle_ctrl_down_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    (void)cx;
    handle_ctrl_key_down(cy);
}

static void handle_help_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    show_help();
    redraw(cx, cy);
}

static void handle_about_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    show_about();
    redraw(cx, cy);
}

static void handle_find_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    find(1);
    redraw(cx, cy);
}

static void handle_delete_line_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    (void)cx;
    delete_current_line(cy, &start_line);
}

static void handle_insert_line_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    insert_new_line(cx, cy, &start_line);
}

static void handle_move_forward_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    move_forward_to_next_word(cx, cy);
}

static void handle_move_backward_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    move_backward_to_previous_word(cx, cy);
}

static void handle_load_file_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    load_file(NULL);
    redraw(cx, cy);
}

static void handle_save_as_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    save_file_as();
    redraw(cx, cy);
}

static void handle_save_file_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    save_file();
    redraw(cx, cy);
}

static void handle_selection_mode_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    if (selection_mode) {
        end_selection_mode();
    } else {
        start_selection_mode(*cx, *cy);
    }
}

static void handle_paste_clipboard_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    paste_clipboard(cx, cy);
}

static void handle_clear_buffer_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    clear_text_buffer();
    *cx = 1;
    *cy = 1;
}

static void handle_redo_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    (void)cx;
    (void)cy;
    redo();
}

static void handle_undo_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    (void)cx;
    (void)cy;
    undo();
}

static void handle_quit_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    (void)cx;
    (void)cy;
    exiting = 1;
}

#define MAX_KEY_MAPPINGS 64
static KeyMapping key_mappings[MAX_KEY_MAPPINGS];
static int key_mapping_count = 0;

static void initialize_key_mappings() {
    key_mapping_count = 0;

    key_mappings[key_mapping_count++] = (KeyMapping){KEY_UP, handle_key_up_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){KEY_DOWN, handle_key_down_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){KEY_LEFT, handle_key_left_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){KEY_RIGHT, handle_key_right_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){KEY_BACKSPACE, handle_key_backspace_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){127, handle_key_backspace_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){KEY_DC, handle_key_delete_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){'\n', handle_key_enter_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){KEY_PPAGE, handle_key_page_up_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){KEY_NPAGE, handle_key_page_down_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){KEY_CTRL_LEFT, handle_ctrl_left_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){KEY_CTRL_RIGHT, handle_ctrl_right_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){KEY_CTRL_PGUP, handle_ctrl_pgup_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){KEY_CTRL_PGDN, handle_ctrl_pgdn_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){KEY_CTRL_UP, handle_ctrl_up_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){KEY_CTRL_DOWN, handle_ctrl_down_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_help, handle_help_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_about, handle_about_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_find, handle_find_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_delete_line, handle_delete_line_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_insert_line, handle_insert_line_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_move_forward, handle_move_forward_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_move_backward, handle_move_backward_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_load_file, handle_load_file_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_save_as, handle_save_as_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_save_file, handle_save_file_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_selection_mode, handle_selection_mode_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_paste_clipboard, handle_paste_clipboard_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_clear_buffer, handle_clear_buffer_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_redo, handle_redo_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_undo, handle_undo_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_quit, handle_quit_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){KEY_CTRL_T, NULL}; /* placeholder for menu key, handled elsewhere */

    key_mappings[key_mapping_count++] = (KeyMapping){0, NULL}; /* terminator */
}


/**
 * Deletes the current line from the text buffer.
 * 
 * @param cursor_y Pointer to the current y-coordinate of the cursor.
 * @param start_line Pointer to the start line of the text buffer.
 */
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
        (*cursor_y)++;
    } else if (*start_line + *cursor_y > line_count) {
        // Move up if at the end of the document
        if (*cursor_y > 1) {
            (*cursor_y)--;
        } else if (*start_line > 0) {
            (*start_line)--;
        }
    }

    // Clear and redraw the text window
    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(text_win);
}

/**
 * Inserts a new line at the current cursor position.
 * 
 * @param cursor_x Pointer to the current x-coordinate of the cursor.
 * @param cursor_y Pointer to the current y-coordinate of the cursor.
 * @param start_line Pointer to the start line of the text buffer.
 */
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

void find_next_occurrence(const char *word, int *cursor_x, int *cursor_y) {
    int found = 0;
    int lines_per_screen = LINES - 3;  // Lines available in a single screen view
    int middle_line = lines_per_screen / 2; // Calculate middle line position
    int start_search = *cursor_y + start_line;

    // Search from the current cursor position to the end of the document
    for (int line = start_search; line < line_count; ++line) {
        const char *line_text = text_buffer[line];
        const char *found_position = strstr(line == start_search ? line_text + *cursor_x : line_text, word);

        if (found_position != NULL) {
            // Calculate new cursor positions
            *cursor_y = line - start_line + 1;
            *cursor_x = found_position - line_text + 1;

            // Adjust start_line based on document size and found line position
            if (line_count <= lines_per_screen) {
                start_line = 0;  // The entire document fits on one screen
            } else {
                if (line < middle_line) {
                    start_line = 0;  // Avoid scrolling past the top
                } else if (line > line_count - middle_line) {
                    start_line = line_count - lines_per_screen;  // Keep the last line visible
                } else {
                    start_line = line - middle_line;  // Center the found line
                }
            }

            // Update cursor position to the line in the middle of the screen
            *cursor_y = line - start_line + 1;

            found = 1;
            break;
        }
    }

    // If not found, wrap around and search from the start to the initial cursor position
    if (!found) {
        for (int line = 0; line < start_search; ++line) {
            const char *line_text = text_buffer[line];
            const char *found_position = strstr(line_text, word);

            if (found_position != NULL) {
                // Calculate new cursor positions
                *cursor_y = line - start_line + 1;
                *cursor_x = found_position - line_text + 1;

                // Adjust start_line based on document size and found line position
                if (line_count <= lines_per_screen) {
                    start_line = 0;  // The entire document fits on one screen
                } else {
                    if (line < middle_line) {
                        start_line = 0;  // Avoid scrolling past the top
                    } else if (line > line_count - middle_line) {
                        start_line = line_count - lines_per_screen;  // Keep the last line visible
                    } else {
                        start_line = line - middle_line;  // Center the found line
                    }
                }

                // Update cursor position to the line in the middle of the screen
                *cursor_y = line - start_line + 1;

                found = 1;
                break;
            }
        }
    }

    if (!found) {
        mvprintw(LINES - 2, 0, "Word not found.");
    } else {
        mvprintw(LINES - 2, 0, "Found at Line: %d, Column: %d", *cursor_y + start_line + 1, *cursor_x + 1);
    }
    refresh();
    wmove(text_win, *cursor_y, *cursor_x);
    wrefresh(text_win);
}


void find(int new_search)
{
    (void)new_search;
    char *output = malloc(256 * sizeof(char));
    *output = '\0';
    show_find_dialog(output, 20);

    find_next_occurrence(output, &cursor_x, &cursor_y);
}

/**
 * Disables the handling of CTRL-C and CTRL-Z signals.
 * This function ignores the SIGINT (CTRL-C) and SIGTSTP (CTRL-Z) signals.
 */
void disable_ctrl_c_z() {
    // Ignore SIGINT (CTRL-C)
    signal(SIGINT, SIG_IGN);
    // Ignore SIGTSTP (CTRL-Z)
    signal(SIGTSTP, SIG_IGN);
}

/**
 * Initializes the editor by setting up the screen,
 * reading the configuration file, enabling color if specified, and setting up
 * various settings and key mappings.
 */
void initialize() {
    // Allocate clipboard buffer
    clipboard = malloc(CLIPBOARD_SIZE);
    if (clipboard == NULL) {
        fprintf(stderr, "Memory allocation failed for clipboard\n");
        exit(1);
    }
    clipboard[0] = '\0';

    // Initialize the screen
    initscr();

    // Read the configuration file
    read_config_file();

    // Enable color if specified
    if (enable_color) {
        start_color();
    }

    // Set up terminal settings
    cbreak();  // Disable line buffering
    noecho();  // Disable echoing of input characters
    keypad(stdscr, TRUE);  // Enable special keys
    meta(stdscr, TRUE);  // Enable 8-bit control characters

    // Initialize mouse support
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    
    // Set timeout to 100 milliseconds
    timeout(10); 

    // Read the configuration file again (in case it was modified)
    read_config_file();

    // Set up color pairs for syntax highlighting
    if (has_colors() && can_change_color()) {
        start_color();
        init_pair(1, COLOR_WHITE, COLOR_BLUE);     // Background color
        init_pair(2, COLOR_CYAN, COLOR_BLACK);     // Keywords
        init_pair(3, COLOR_GREEN, COLOR_BLACK);    // Comments
        init_pair(4, COLOR_YELLOW, COLOR_BLACK);   // Strings
        init_pair(5, COLOR_MAGENTA, COLOR_BLACK);  // Types
        init_pair(6, COLOR_BLUE, COLOR_BLACK);     // Symbols (braces, parentheses)
    }

    // Set the background color of the screen
    bkgd(COLOR_PAIR(1));

    // Refresh the screen
    refresh();

    // Handle window resizing
    signal(SIGWINCH, handle_resize);

    // Disable handling of CTRL-C and CTRL-Z signals
    disable_ctrl_c_z();

    // Map escape sequences for CTRL-Left and CTRL-Right to custom key constants
    define_key("\033[1;5D", KEY_CTRL_LEFT);  // Escape sequence for CTRL-Left
    define_key("\033[1;5C", KEY_CTRL_RIGHT);  // Escape sequence for CTRL-Right

    // Map escape sequences for CTRL-Page Up and CTRL-Page Down to custom key constants
    define_key("\033[5;5~", KEY_CTRL_PGUP);  // Escape sequence for CTRL-Page Up
    define_key("\033[6;5~", KEY_CTRL_PGDN);  // Escape sequence for CTRL-Page Down

    // Map escape sequences for CTRL-Up and CTRL-Down to custom key constants
    define_key("\033[1;5A", KEY_CTRL_UP);  // Escape sequence for CTRL-Up
    define_key("\033[1;5B", KEY_CTRL_DOWN);  // Escape sequence for CTRL-Down

    // Map CTRL-T to custom key constant
    define_key("\024", KEY_CTRL_T);  // \024 is the octal for CTRL-T

    // Set up key mappings
    initialize_key_mappings();

    // Initialize menus
    initializeMenus();

    // Update the status bar
    update_status_bar(0, 0);
}

/**
 * Runs the editor and handles user input.
 * 
 * This function is responsible for running the editor and handling user input.
 * It continuously listens for key presses and performs the corresponding actions.
 * The function also updates the status bar and refreshes the screen after each action.
 * 
 * @param None
 * @return None
 */
void run_editor() {
    if (runeditor == 0)
        runeditor = 1;
    else
        return;

    int ch;
    int currentMenu = 0;
    int currentItem = 0;
    MEVENT event; // Mouse event structure

    wmove(text_win, cursor_x, cursor_y);

    while ((ch = wgetch(text_win)) && exiting == 0) { // Exit on ESC key
        if (ch == ERR) {
            continue; // Handle any errors or no input case
        }

        if (exiting == 1) {
            break;
        }

        //mvprintw(LINES - 1, 0, "Pressed key: %d", ch); // Add this line for debugging
        drawBar();
        update_status_bar(cursor_x, cursor_y);        
        refresh();
        
        if (selection_mode) {
            handle_selection_mode(ch, &cursor_x, &cursor_y);
        } else if (ch == KEY_CTRL_T) { // CTRL-T
            refresh();
            handleMenuNavigation(menus, menuCount, &currentMenu, &currentItem);
            // Redraw the editor screen after closing the menu
            redraw(&cursor_x, &cursor_y);
        } else if (ch == KEY_MOUSE) {
            if (getmouse(&event) == OK) {
                // Check if the mouse event is a click within the text window
                if (event.bstate & BUTTON1_PRESSED) {
                    int new_x = event.x;
                    int new_y = event.y;

                    // Convert the mouse position to cursor position
                    if (new_x < COLS - 2 && new_y < LINES - 3) {
                        cursor_x = new_x;
                        cursor_y = new_y - 1;
                    }
                }
            }
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

/**
 * Cleans up the text buffer and frees allocated memory on program exit.
 * 
 * This function is responsible for cleaning up the text buffer and freeing
 * the allocated memory for each line of text. It is called when the program
 * exits to ensure that all memory is properly deallocated.
 * 
 * @param None
 * @return None
 */
void cleanup_on_exit() {
    for (int i = 0; i < MAX_LINES; ++i) {
        if (text_buffer[i] != NULL) {  // Check for NULL pointer
            free(text_buffer[i]);  // Free the memory allocated for the line of text
            text_buffer[i] = NULL;  // Set to NULL to avoid double free
        }
    }

    if (clipboard != NULL) {
        free(clipboard);
        clipboard = NULL;
    }

    freeMenus();
}

void close_editor() {
    exiting = 1;
}

/**
 * Initializes the text buffer by allocating memory for each line and setting initial values.
 * 
 * This function is responsible for initializing the text buffer by allocating memory for each line
 * and setting initial values such as line count and start line. It is called during the editor's
 * initialization process to ensure that the text buffer is properly set up before any editing
 * operations are performed.
 * 
 * @param None
 * @return None
 */
void initialize_buffer() {
    // Allocate memory for each line in the text buffer
    for (int i = 0; i < MAX_LINES; ++i) {
        if (text_buffer[i] != NULL) {
            free(text_buffer[i]);
            text_buffer[i] = NULL;
        }
        text_buffer[i] = (char *)calloc(COLS - 3, sizeof(char));
        if (text_buffer[i] == NULL) {
            // Handle allocation failure
            fprintf(stderr, "Memory allocation failed for text_buffer[%d]\n", i);
            exit(1);
        }
    }
    
    // Set the initial line count to 1
    line_count = 1;
    
    // Set the initial start line to 0
    start_line = 0;
}

/**
 * Draws the text buffer on the specified window.
 * 
 * This function is responsible for drawing the contents of the text buffer on the specified window.
 * It applies syntax highlighting to each line of text and also draws a scrollbar to indicate the
 * current position in the document. The function takes into account the start line and the maximum
 * number of lines that can be displayed on the window to determine which lines to draw.
 * 
 * @param win The window on which to draw the text buffer.
 * @return None
 */
void draw_text_buffer(WINDOW *win) {
    if (start_line < 0)
        start_line = 0;

    werase(win);
    box(win, 0, 0);
    int max_lines = LINES - 4;  // Adjust for the status bar

    // Iterate over each line to be displayed on the window
    for (int i = 0; i < max_lines && i + start_line < line_count; ++i) {
        // Apply syntax highlighting to the current line of text
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
            mvwprintw(win, i + 1, COLS - 1, "|");
        } else {
            mvwprintw(win, i + 1, COLS - 1, " ");
        }
    }

    // Draw scrollbar at the top if the cursor is at the very top of the document
    if (start_line == 0 && scrollbar_start == 0 && scrollbar_end > 0) {
        mvwprintw(win, 1, COLS - 1, "|");
    }

    wrefresh(win);
}

/**
 * This function handles the regular mode of the editor, where the user can navigate and edit the text.
 * It takes a character input 'ch' and the current cursor position 'cursor_x' and 'cursor_y' as parameters.
 * Based on the input, it performs various actions such as moving the cursor, deleting characters, inserting new lines, etc.
 * 
 * @param ch The character input from the user.
 * @param cursor_x The current x-coordinate of the cursor.
 * @param cursor_y The current y-coordinate of the cursor.
 * @return None
 */
void handle_regular_mode(int ch, int *cursor_x, int *cursor_y) {
    for (int i = 0; i < key_mapping_count; ++i) {
        if (key_mappings[i].key == 0 && key_mappings[i].handler == NULL) {
            break;
        }
        if (key_mappings[i].key == ch) {
            if (key_mappings[i].handler) {
                key_mappings[i].handler(NULL, cursor_x, cursor_y);
            }
            return;
        }
    }

    handle_default_key(ch, cursor_x, *cursor_y);
}

/**
 * Redraws the editor screen after a resize event.
 * 
 * This function clears the text window, redraws the text buffer, and moves the cursor to its previous position.
 * 
 * @param cursor_x A pointer to the current x-coordinate of the cursor.
 * @param cursor_y A pointer to the current y-coordinate of the cursor.
 * @return None
 */
void redraw(int *cursor_x, int *cursor_y) {
    werase(text_win); // Clear the text window
    box(text_win, 0, 0); // Redraw the border of the text window
    draw_text_buffer(text_win); // Redraw the text buffer
    wmove(text_win, *cursor_y, *cursor_x); // Move the cursor to its previous position
    wrefresh(text_win); // Refresh the text window
}

/**
 * Handles the resize signal.
 * 
 * This function is called when the terminal window is resized. It clears the screen, redraws the interface,
 * and updates the status bar.
 * 
 * @param sig The signal number.
 * @return None
 */
void handle_resize(int sig) {
    (void)sig; // Cast to void to suppress unused parameter warning

    endwin(); // End the curses mode
    refresh(); // Refresh the screen
    clear(); // Clear the screen

    // Redraw the interface
    werase(text_win); // Clear the text window
    wresize(text_win, LINES - 2, COLS); // Resize the text window
    mvwin(text_win, 1, 0); // Move the text window to its new position
    box(text_win, 0, 0); // Redraw the border of the text window
    draw_text_buffer(text_win); // Redraw the text buffer
    wrefresh(text_win); // Refresh the text window

    update_status_bar(1, 1); // Update the status bar with some default values for cursor position
}

/**
 * Clears the text buffer.
 * 
 * This function clears the text buffer by setting all elements to 0 and resets the line count and start line variables.
 * It also clears the text window, redraws the border, and refreshes the window.
 * 
 * @return None
 */
void clear_text_buffer() {
    // Set all elements of the text buffer to 0
    for (int i = 0; i < MAX_LINES; ++i) {
        memset(text_buffer[i], 0, COLS - 3);
    }
    
    // Reset line count and start line variables
    line_count = 1;
    start_line = 1;
    
    // Clear the text window
    werase(text_win);
    
    // Redraw the border of the text window
    box(text_win, 0, 0);
    
    // Refresh the text window
    wrefresh(text_win);
}

/**
 * Saves the current file.
 * 
 * If the current file has a filename, it saves the file with the existing filename.
 * If the current file does not have a filename, it prompts the user to enter a filename and saves the file with the new name.
 * 
 * @return None
 */
void save_file() {
    // Check if the current file has a filename
    if (strlen(current_filename) == 0) {
        save_file_as(); // If the current file does not have a filename, prompt the user to enter a filename and save the file with the new name
    } else {
        FILE *fp = fopen(current_filename, "w"); // Open the file for writing
        if (fp) {
            // Iterate over each line in the text buffer and write it to the file
            for (int i = 0; i < line_count; ++i) {
                fprintf(fp, "%s\n", text_buffer[i]);
            }
            fclose(fp); // Close the file
            mvprintw(LINES - 2, 2, "File saved as %s", current_filename); // Display a success message
        } else {
            mvprintw(LINES - 2, 2, "Error saving file!"); // Display an error message if the file cannot be opened
        }
        refresh();
        getch();
        mvprintw(LINES - 2, 2, "                            "); // Clear the line after saving
        refresh();
    }
}

/**
 * Saves the current file with a new name.
 * 
 * This function prompts the user to enter a filename and saves the contents of the text buffer to the file.
 * If the file is successfully saved, a success message is displayed. Otherwise, an error message is displayed.
 * 
 * @return None
 */
void save_file_as() {
    // Prompt the user to enter a filename
    create_dialog("Save as", current_filename, 256);

    // Open the file for writing
    FILE *fp = fopen(current_filename, "w");
    if (fp) {
        // Iterate over each line in the text buffer and write it to the file
        for (int i = 0; i < line_count; ++i) {
            fprintf(fp, "%s\n", text_buffer[i]);
        }
        fclose(fp); // Close the file
        mvprintw(LINES - 2, 2, "File saved as %s", current_filename); // Display a success message
    } else {
        mvprintw(LINES - 2, 2, "Error saving file!"); // Display an error message if the file cannot be opened
    }

    refresh();
    getch();
    mvprintw(LINES - 2, 2, "                            "); // Clear the line after saving
    refresh();
}

/**
 * Loads a file into the text buffer and displays it in the editor.
 * 
 * This function loads the contents of a file into the text buffer and displays it in the editor.
 * If a filename is provided, it loads that file. Otherwise, it prompts the user to enter a filename.
 * It also sets the syntax mode based on the file extension and initializes the text buffer.
 * After loading the file, it updates the status bar with a success or error message.
 * 
 * @param filename The name of the file to load. If NULL, the user will be prompted to enter a filename.
 * @return None
 */
void load_file(const char *filename) {
    char file_to_load[256];

    // If no filename is provided, prompt the user
    if (filename == NULL) {
        create_dialog("Load file", file_to_load, 256);
        filename = file_to_load;
    }

    set_syntax_mode(filename); // Set the syntax mode based on the file extension
    initialize_buffer(); // Initialize the text buffer

    FILE *fp = fopen(filename, "r"); // Open the file for reading
    if (fp) {
        line_count = 0; // Initialize the line count to 0
        while (fgets(text_buffer[line_count], COLS - 3, fp) && line_count < MAX_LINES) {
            // Remove newline character if present
            text_buffer[line_count][strcspn(text_buffer[line_count], "\n")] = '\0';
            line_count++; // Increment the line count
        }
        fclose(fp); // Close the file
        mvprintw(LINES - 2, 2, "File loaded: %s", filename); // Display a success message

        strcpy(current_filename, filename); // Copy the loaded filename to the current filename variable
    } else {
        mvprintw(LINES - 2, 2, "Error loading file!"); // Display an error message if the file cannot be opened
    }

    refresh(); // Refresh the screen

    // Wait for a brief moment to display the message, without requiring Enter
    timeout(100); // Wait for 100 milliseconds
    getch(); // Consume any key press if available
    timeout(-1); // Reset to blocking mode

    mvprintw(LINES - 2, 2, "                            "); // Clear the line after loading
    refresh(); // Refresh the screen
    text_win = newwin(LINES - 2, COLS, 1, 0); // Create a new text window
    keypad(text_win, TRUE); // Enable keypad mode for the text window
    meta(text_win, TRUE); // Enable meta keys for the text window

    box(text_win, 0, 0); // Draw a border around the text window
    wmove(text_win, 1, 1); // Move the cursor to the initial position

    draw_text_buffer(text_win); // Draw the text buffer in the text window
    wrefresh(text_win); // Refresh the text window
}

/**
 * Updates the status bar with the current cursor position and other information.
 * 
 * This function displays the filename centered on line 2, the line count, current line number,
 * and column number on the bottom line of the screen. It also displays the "CTRL-H - Help" message
 * on the bottom right corner of the screen.
 * 
 * @param cursor_y The current y-coordinate of the cursor.
 * @param cursor_x The current x-coordinate of the cursor.
 * @return None
 */
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

/**
 * Creates a new file.
 * 
 * This function initializes the necessary variables and data structures to start editing a new file.
 * It also creates a new text window, sets up the necessary key bindings, and refreshes the window.
 * 
 * @return None
 */
void new_file() {
    int cursor_x = 1, cursor_y = 1; // Initialize the cursor position
    strcpy(current_filename, ""); // Set the current filename to an empty string

    initialize_buffer(); // Initialize the text buffer

    text_win = newwin(LINES - 2, COLS, 1, 0); // Create a new text window
    keypad(text_win, TRUE); // Enable keypad mode for the text window
    meta(text_win, TRUE); // Enable meta keys for the text window
    box(text_win, 0, 0); // Draw a border around the text window
    wmove(text_win, cursor_y, cursor_x); // Move the cursor to the initial position
    wrefresh(text_win); // Refresh the text window
}

/**
 * Sets the syntax mode based on the file extension.
 * 
 * This function takes a filename as input and determines the syntax mode based on its extension.
 * If the extension matches a known programming language, the corresponding syntax mode is set.
 * If the extension is not recognized, the syntax mode is set to NO_SYNTAX.
 * 
 * @param filename The name of the file.
 * @return None
 */
void set_syntax_mode(const char *filename) {
    const char *ext = strrchr(filename, '.'); // Get the file extension
    if (ext) {
        if (strcmp(ext, ".c") == 0 || strcmp(ext, ".h") == 0) { // Check if the extension is .c or .h
            current_syntax_mode = C_SYNTAX; // Set the syntax mode to C_SYNTAX
        } else if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) { // Check if the extension is .html or .htm
            current_syntax_mode = HTML_SYNTAX; // Set the syntax mode to HTML_SYNTAX
        } else if (strcmp(ext, ".py") == 0) { // Check if the extension is .py
            current_syntax_mode = PYTHON_SYNTAX; // Set the syntax mode to PYTHON_SYNTAX
        } else if (strcmp(ext, ".cs") == 0) { // Check if the extension is .cs
            current_syntax_mode = CSHARP_SYNTAX; // Set the syntax mode to CSHARP_SYNTAX
        } else {
            current_syntax_mode = NO_SYNTAX; // Set the syntax mode to NO_SYNTAX for unrecognized extensions
        }
    } else {
        current_syntax_mode = NO_SYNTAX; // Set the syntax mode to NO_SYNTAX if no extension is found
    }
}
