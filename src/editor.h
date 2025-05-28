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

#define MAX_LINES 5000

// Define custom key constants for CTRL-Left, CTRL-Right, CTRL-Page Up, CTRL-Page Down
#define KEY_CTRL_LEFT       1000
#define KEY_CTRL_RIGHT      1001
#define KEY_CTRL_PGUP       1002
#define KEY_CTRL_PGDN       1003
#define KEY_CTRL_UP         1004
#define KEY_CTRL_DOWN       1005
#define KEY_CTRL_Q          1007
#define KEY_CTRL_BACKTICK   30
#define KEY_CTRL_T          20

struct FileState;
typedef void (*KeyHandler)(struct FileState *, int *cx, int *cy);

typedef struct {
    int key;
    KeyHandler handler;
} KeyMapping;

extern WINDOW *text_win;
extern struct FileState *active_file;
struct FileManager;
extern struct FileManager file_manager;
void fm_init(struct FileManager *fm);
struct FileState *fm_current(struct FileManager *fm);
int fm_add(struct FileManager *fm, struct FileState *fs);
void fm_close(struct FileManager *fm, int index);
int fm_switch(struct FileManager *fm, int index);
void handle_regular_mode(struct FileState *fs, int ch);
void initialize(void);
void draw_text_buffer(struct FileState *fs, WINDOW *win);
void close_editor(void);
void clear_text_buffer(void);
void run_editor(void);
void initialize_buffer(void);
void redraw(void);
void delete_current_line(struct FileState *fs);
void insert_new_line(struct FileState *fs);
void update_status_bar(struct FileState *fs);
void handle_resize(int sig);
void cleanup_on_exit(void);
void disable_ctrl_c_z(void);
void next_file(struct FileState *fs, int *cx, int *cy);
void prev_file(struct FileState *fs, int *cx, int *cy);


#endif // EDITOR_H
