#ifndef EDITOR_H
#define EDITOR_H

#include <ncurses.h>
#include <string.h>

typedef struct Change {
    int line;
    char *old_text;
    char *new_text;
} Change;

typedef struct Node {
    Change change;
    struct Node *next;
} Node;

#define MAX_LINES 1000

// Define custom key constants for CTRL-Left, CTRL-Right, CTRL-Page Up, CTRL-Page Down
#define KEY_CTRL_LEFT       1000
#define KEY_CTRL_RIGHT      1001
#define KEY_CTRL_PGUP       1002
#define KEY_CTRL_PGDN       1003
#define KEY_CTRL_UP         1004
#define KEY_CTRL_DOWN       1005
#define KEY_CTRL_Q          1007
#define KEY_CTRL_BACKTICK   30

void initialize();
void draw_text_buffer(WINDOW *win);
void clear_text_buffer();
void run_editor();
void initialize_buffer();
void save_file();
void save_file_as();
void load_file();
void new_file();
void undo();
void redo();
void update_status_bar(int cursor_y, int cursor_x);
void handle_resize(int sig);

// Stack functions for undo and redo
void push(Node **stack, Change change);
Change pop(Node **stack);
int is_empty(Node *stack);
void free_stack(Node *stack);

extern WINDOW *text_win;
extern char *text_buffer[MAX_LINES];
extern int line_count;
extern int start_line;
extern Node *undo_stack;
extern Node *redo_stack;

#endif
