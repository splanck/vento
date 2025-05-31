#ifndef EDITOR_H
#define EDITOR_H

#include <ncurses.h>
#include <signal.h>
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


#define DEFAULT_BUFFER_LINES 5000

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

void initialize_key_mappings(void);

extern WINDOW *text_win;
extern struct FileState *active_file;
extern struct FileManager file_manager;
extern char search_text[256];
extern volatile sig_atomic_t resize_pending;
extern int exiting;
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
void go_to_line(struct FileState *fs, int line);
__attribute__((weak)) int get_line_number_offset(struct FileState *fs);
void on_sigwinch(int sig);
void perform_resize(void);
void cleanup_on_exit(struct FileManager *fm);
void disable_ctrl_c_z(void);
void apply_colors(void);
void next_file(struct FileState *fs, int *cx, int *cy);
void prev_file(struct FileState *fs, int *cx, int *cy);
void handle_redo_wrapper(struct FileState *fs, int *cx, int *cy);
void handle_undo_wrapper(struct FileState *fs, int *cx, int *cy);
void allocation_failed(const char *msg);


#endif // EDITOR_H
