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
#include "file_ops.h"
#include "search.h"
#include "file_manager.h"

FileState *active_file = NULL;
/* text_buffer and line_count are now stored per file in FileState */
int runeditor = 0;  // Flag to control the main loop of the editor
WINDOW *text_win;  // Pointer to the ncurses window for displaying the text

// Undo and redo stacks are stored per file in FileState

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
int key_close_file = 17;  // Key code for closing the current file (CTRL-Q)
int key_selection_mode = 13;  // Key code for entering selection mode
int key_paste_clipboard = 11;  // Key code for pasting from clipboard
int key_clear_buffer = 14;  // Key code for clearing the text buffer
int key_redo = 18;  // Key code for the redo command
int key_undo = 21;  // Key code for the undo command
int key_quit = 24;  // Key code for quitting the editor
int key_find = 6;  // Key code for finding next word
int key_next_file = KEY_F(6);  // Key code for switching to the next file
int key_prev_file = KEY_F(7);  // Key code for switching to the previous file

static void handle_key_up_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    handle_key_up(fs);
}

static void handle_key_down_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    handle_key_down(fs);
}

static void handle_key_left_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    handle_key_left(fs);
}

static void handle_key_right_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    handle_key_right(fs);
}

static void handle_key_backspace_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    handle_key_backspace(fs);
}

static void handle_key_delete_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    handle_key_delete(fs);
}

static void handle_key_enter_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    handle_key_enter(fs);
}

static void handle_key_page_up_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    handle_key_page_up(fs);
}

static void handle_key_page_down_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    handle_key_page_down(fs);
    redraw();
}

static void handle_ctrl_left_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    handle_ctrl_key_left(fs);
}

static void handle_ctrl_right_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    handle_ctrl_key_right(fs);
}

static void handle_ctrl_pgup_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    handle_ctrl_key_pgup(fs);
}

static void handle_ctrl_pgdn_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    handle_ctrl_key_pgdn(fs);
}

static void handle_ctrl_up_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    handle_ctrl_key_up(fs);
}

static void handle_ctrl_down_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    handle_ctrl_key_down(fs);
}

static void handle_help_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    (void)cx;
    (void)cy;
    show_help();
    redraw();
}

static void handle_about_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    (void)cx;
    (void)cy;
    show_about();
    redraw();
}

static void handle_find_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    find(fs, 1);
    redraw();
}

static void handle_delete_line_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    delete_current_line(fs);
    *cy = fs->cursor_y;
}

static void handle_insert_line_wrapper(struct FileState *fs, int *cx, int *cy) {
    insert_new_line(fs);
    *cx = fs->cursor_x;
    *cy = fs->cursor_y;
}

static void handle_move_forward_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    move_forward_to_next_word(fs);
}

static void handle_move_backward_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    move_backward_to_previous_word(fs);
}

static void handle_load_file_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    load_file(fs, NULL);
    redraw();
}

static void handle_save_as_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    save_file_as(fs);
    redraw();
}

static void handle_save_file_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    save_file(fs);
    redraw();
}

static void handle_selection_mode_wrapper(struct FileState *fs, int *cx, int *cy) {
    if (fs->selection_mode) {
        end_selection_mode(fs);
    } else {
        start_selection_mode(fs, *cx, *cy);
    }
}

static void handle_paste_clipboard_wrapper(struct FileState *fs, int *cx, int *cy) {
    paste_clipboard(fs, cx, cy);
}

static void handle_clear_buffer_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    clear_text_buffer();
    *cx = 1;
    *cy = 1;
}

static void handle_redo_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    redo(fs);
}

static void handle_undo_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)cx;
    (void)cy;
    undo(fs);
}

static void handle_quit_wrapper(struct FileState *fs, int *cx, int *cy) {
    (void)fs;
    (void)cx;
    (void)cy;
    exiting = 1;
}

void next_file(FileState *fs_unused, int *cx, int *cy) {
    (void)fs_unused;
    if (file_manager.count == 0) {
        return;
    }

    int idx = file_manager.active_index + 1;
    if (idx >= file_manager.count) idx = 0;
    fm_switch(&file_manager, idx);
    active_file = fm_current(&file_manager);
    text_win = active_file->text_win;

    *cx = active_file->cursor_x;
    *cy = active_file->cursor_y;
    redraw();
    update_status_bar(active_file);
}

void prev_file(FileState *fs_unused, int *cx, int *cy) {
    (void)fs_unused;
    if (file_manager.count == 0) {
        return;
    }

    int idx = file_manager.active_index - 1;
    if (idx < 0) idx = file_manager.count - 1;
    fm_switch(&file_manager, idx);
    active_file = fm_current(&file_manager);
    text_win = active_file->text_win;

    *cx = active_file->cursor_x;
    *cy = active_file->cursor_y;
    redraw();
    update_status_bar(active_file);
}

#define MAX_KEY_MAPPINGS 64
static KeyMapping key_mappings[MAX_KEY_MAPPINGS];
static int key_mapping_count = 0;

static void initialize_key_mappings(void) {
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
    key_mappings[key_mapping_count++] = (KeyMapping){key_close_file, close_current_file};
    key_mappings[key_mapping_count++] = (KeyMapping){key_selection_mode, handle_selection_mode_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_paste_clipboard, handle_paste_clipboard_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_clear_buffer, handle_clear_buffer_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_redo, handle_redo_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_undo, handle_undo_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){key_next_file, next_file};
    key_mappings[key_mapping_count++] = (KeyMapping){key_prev_file, prev_file};
    key_mappings[key_mapping_count++] = (KeyMapping){key_quit, handle_quit_wrapper};
    key_mappings[key_mapping_count++] = (KeyMapping){KEY_CTRL_T, NULL}; /* placeholder for menu key, handled elsewhere */

    key_mappings[key_mapping_count++] = (KeyMapping){0, NULL}; /* terminator */
}


/**
 * Deletes the current line from the text buffer.
 * 
 * @param fs Pointer to the current file state.
 */
void delete_current_line(FileState *fs) {
    if (fs->line_count == 0) {
        return;
    }

    int line_to_delete = fs->cursor_y - 1 + fs->start_line;

    // Push the current state to the undo stack
    push(&fs->undo_stack,
         (Change){line_to_delete, strdup(fs->text_buffer[line_to_delete]), NULL});

    // Shift lines up to delete the current line
    for (int i = line_to_delete; i < fs->line_count - 1; ++i) {
        strcpy(fs->text_buffer[i], fs->text_buffer[i + 1]);
    }

    // Clear the last line
    memset(fs->text_buffer[fs->line_count - 1], 0, COLS - 3);
    fs->line_count--;

    // Move cursor to the next line if possible
    if (fs->cursor_y < LINES - 4 && fs->cursor_y <= fs->line_count) {
        // Move to the next line
        fs->cursor_y++;
    } else if (fs->start_line + fs->cursor_y > fs->line_count) {
        // Move up if at the end of the document
        if (fs->cursor_y > 1) {
            fs->cursor_y--;
        } else if (fs->start_line > 0) {
            fs->start_line--;
        }
    }

    // Clear and redraw the text window
    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(fs, text_win);
    mark_comment_state_dirty(fs);
}

/**
 * Inserts a new line at the current cursor position.
 * 
 * @param fs Pointer to the current file state.
 */
void insert_new_line(FileState *fs) {
    ensure_line_capacity(fs, fs->line_count + 1);
    // Move lines below the cursor down by one
    for (int i = fs->line_count; i > fs->cursor_y + fs->start_line - 1; --i) {
        strcpy(fs->text_buffer[i], fs->text_buffer[i - 1]);
    }
    fs->line_count++;

    // Insert a new empty line at the current cursor position
    fs->text_buffer[fs->cursor_y + fs->start_line - 1][0] = '\0';

    // Record the change for undo
    Change change;
    change.line = fs->cursor_y + fs->start_line - 1;
    change.old_text = NULL;
    change.new_text = strdup("");

    push(&fs->undo_stack, change);
    mark_comment_state_dirty(fs);

    // Move cursor to the new line
    fs->cursor_x = 1;

    // Adjust cursor_y and start_line
    if (fs->cursor_y == LINES - 4 && fs->start_line + LINES - 4 < fs->line_count) {
        fs->start_line++;
    } else {
        fs->cursor_y++;
    }

    // Clear and redraw the text window
    redraw();

    // Move the cursor up one line
    if (fs->cursor_y > 1) {
        fs->cursor_y--;
    } else if (fs->start_line > 0) {
        fs->start_line--;
    }
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
    // Initialize the screen
    initscr();

    // Load the configuration file
    config_load(&app_config);

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
    update_status_bar(active_file);
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

    wmove(text_win, active_file->cursor_y, active_file->cursor_x);

    while ((ch = wgetch(text_win)) && exiting == 0) { // Exit on ESC key
        if (ch == ERR) {
            continue; // Handle any errors or no input case
        }

        if (exiting == 1) {
            break;
        }

        //mvprintw(LINES - 1, 0, "Pressed key: %d", ch); // Add this line for debugging
        drawBar();
        update_status_bar(active_file);
        refresh();
        
        if (active_file->selection_mode) {
            handle_selection_mode(active_file, ch, &active_file->cursor_x, &active_file->cursor_y);
        } else if (ch == KEY_CTRL_T) { // CTRL-T
            refresh();
            handleMenuNavigation(menus, menuCount, &currentMenu, &currentItem);
            // Redraw the editor screen after closing the menu
            redraw();
        } else if (ch == KEY_MOUSE) {
            if (getmouse(&event) == OK) {
                // Check if the mouse event is a click within the text window
                if (event.bstate & BUTTON1_PRESSED) {
                    int new_x = event.x;
                    int new_y = event.y;

                    // Convert the mouse position to cursor position
                    if (new_x < COLS - 2 && new_y < LINES - 3) {
                        active_file->cursor_x = new_x;
                        active_file->cursor_y = new_y - 1;
                    }
                }
            }
        } else {
            handle_regular_mode(active_file, ch);
        }

        if (exiting == 1)
            break;

        update_status_bar(active_file);
        wmove(text_win, active_file->cursor_y, active_file->cursor_x);  // Restore cursor position
        wrefresh(text_win);
    }

    delwin(text_win);
}

/**
 * Cleans up all open files and menus before program exit.
 *
 * Iterates through all files managed by @p fm, freeing undo/redo stacks and
 * each FileState. Menu structures are also released.
 *
 * @param fm Pointer to the FileManager containing open files.
 */
void cleanup_on_exit(FileManager *fm) {
    if (!fm || !fm->files) {
        freeMenus();
        return;
    }

    for (int i = 0; i < fm->count; ++i) {
        FileState *fs = fm->files[i];
        if (!fs) continue;

        free_stack(fs->undo_stack);
        fs->undo_stack = NULL;
        free_stack(fs->redo_stack);
        fs->redo_stack = NULL;

        free_file_state(fs, fs->max_lines);
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
    for (int i = 0; i < active_file->max_lines; ++i) {
        if (active_file->text_buffer[i] != NULL) {
            free(active_file->text_buffer[i]);
            active_file->text_buffer[i] = NULL;
        }
        active_file->text_buffer[i] = (char *)calloc(COLS - 3, sizeof(char));
        if (active_file->text_buffer[i] == NULL) {
            // Handle allocation failure
            fprintf(stderr, "Memory allocation failed for text_buffer[%d]\n", i);
            exit(1);
        }
    }
    
    // Set the initial line count to 1
    active_file->line_count = 1;
    
    // Set the initial start line to 0
    if (active_file)
        active_file->start_line = 0;

    // Allocate clipboard if needed
    if (active_file && active_file->clipboard == NULL) {
        active_file->clipboard = malloc(CLIPBOARD_SIZE);
        if (!active_file->clipboard) {
            fprintf(stderr, "Memory allocation failed for clipboard\n");
            exit(1);
        }
        active_file->clipboard[0] = '\0';
    }
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
void draw_text_buffer(FileState *fs, WINDOW *win) {
    if (fs->start_line < 0)
        fs->start_line = 0;

    sync_multiline_comment(fs, fs->start_line);

    werase(win);
    box(win, 0, 0);
    int max_lines = LINES - 4;  // Adjust for the status bar

    // Iterate over each line to be displayed on the window
    for (int i = 0; i < max_lines && i + fs->start_line < fs->line_count; ++i) {
        // Apply syntax highlighting to the current line of text
        apply_syntax_highlighting(fs, win, fs->text_buffer[i + fs->start_line], i + 1);
    }

    // Calculate scrollbar position and size
    int scrollbar_height = max_lines;
    int scrollbar_start = 0;
    int scrollbar_end = 0;

    if (fs->line_count > 0) {
        scrollbar_start = (fs->start_line * scrollbar_height) / fs->line_count;
        scrollbar_end = ((fs->start_line + max_lines) * scrollbar_height) / fs->line_count;
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
    if (fs->start_line == 0 && scrollbar_start == 0 && scrollbar_end > 0) {
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
void handle_regular_mode(FileState *fs, int ch) {
    for (int i = 0; i < key_mapping_count; ++i) {
        if (key_mappings[i].key == 0 && key_mappings[i].handler == NULL) {
            break;
        }
        if (key_mappings[i].key == ch) {
            if (key_mappings[i].handler) {
                key_mappings[i].handler(fs, &fs->cursor_x, &fs->cursor_y);
            }
            return;
        }
    }

    handle_default_key(fs, ch);
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
void redraw() {
    werase(text_win); // Clear the text window
    box(text_win, 0, 0); // Redraw the border of the text window
    draw_text_buffer(active_file, text_win); // Redraw the text buffer
    wmove(text_win, active_file->cursor_y, active_file->cursor_x); // Move the cursor to its previous position
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

    /* Resize all open file windows */
    for (int i = 0; i < file_manager.count; ++i) {
        FileState *fs = file_manager.files[i];
        if (!fs || !fs->text_win) {
            continue;
        }
        wresize(fs->text_win, LINES - 2, COLS);
        mvwin(fs->text_win, 1, 0);
    }

    /* Use the resized window of the active file */
    text_win = active_file->text_win;
    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(active_file, text_win);
    wrefresh(text_win);

    update_status_bar(active_file);
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
    for (int i = 0; i < active_file->max_lines; ++i) {
        memset(active_file->text_buffer[i], 0, COLS - 3);
    }

    // Reset line count and start line variables
    active_file->line_count = 1;
    if (active_file)
        active_file->start_line = 0;
    
    // Clear the text window
    werase(text_win);
    
    // Redraw the border of the text window
    box(text_win, 0, 0);
    
    // Refresh the text window
    wrefresh(text_win);
}


void update_status_bar(FileState *fs) {
    move(0, 0);

    int idx = file_manager.active_index + 1;
    int total = file_manager.count > 0 ? file_manager.count : 1;

    const char *name = "untitled";
    if (fs && fs->filename[0] != '\0') {
        name = fs->filename;
    }
    char display[512];
    snprintf(display, sizeof(display), "%s [%d/%d]", name, idx, total);

    int center_position = (COLS - (int)strlen(display)) / 2;
    mvprintw(1, center_position, "%s", display);

    move(LINES - 1, 0);
    clrtoeol();
    int actual_line_number = fs ? (fs->cursor_y + fs->start_line) : 0;
    mvprintw(LINES - 1, 0, "Lines: %d  Current Line: %d  Column: %d", fs ? fs->line_count : 0, actual_line_number, fs ? fs->cursor_x : 0);

    mvprintw(LINES - 1, COLS - 15, "CTRL-H - Help");

    refresh();
}
