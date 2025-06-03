#ifndef EDITOR_H
#define EDITOR_H

#include <ncurses.h>
#include <signal.h>
#include <string.h>
#include <wchar.h>
typedef struct EditorContext EditorContext;

/*
 * Undo/Redo Stack
 * ---------------
 * Each file maintains an undo and redo history implemented as a singly linked
 * list.  A `Node` represents one entry in these stacks and contains a `Change`
 * describing the modification to a single line.
 *
 * A `Change` stores the line index along with the previous and new contents of
 * that line.  When `old_text` is NULL the change represents an insertion and
 * when `new_text` is NULL it represents a deletion.  Both strings are
 * dynamically allocated and ownership is transferred to the stack when the
 * change is pushed.  They are freed when the entry is popped or when the entire
 * stack is destroyed.
 */
typedef struct Change {
    int line;        /* Affected line index */
    char *old_text;  /* Text before the change or NULL */
    char *new_text;  /* Text after the change or NULL */
} Change;

typedef struct Node {
    Change change;   /* Stored modification */
    struct Node *next; /* Next node in the stack */
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
struct Macro;
typedef void (*KeyHandler)(struct FileState *, int *cx, int *cy);

typedef struct {
    int key;
    KeyHandler handler;
} KeyMapping;

void initialize_key_mappings(void);

extern WINDOW *text_win;
extern struct FileState *active_file;
extern struct FileManager file_manager;
extern struct Macro *current_macro;
extern char search_text[256];
extern volatile sig_atomic_t resize_pending;
extern int exiting;
extern EditorContext editor;
extern int start_line;
void handle_regular_mode(EditorContext *ctx, struct FileState *fs, wint_t ch);
void initialize(EditorContext *ctx);
void draw_text_buffer(struct FileState *fs, WINDOW *win);
void close_editor(void);
void clear_text_buffer(void);
void run_editor(EditorContext *ctx);
void initialize_buffer(void);
void redraw(void);
void delete_current_line(EditorContext *ctx, struct FileState *fs);
void insert_new_line(EditorContext *ctx, struct FileState *fs);
void update_status_bar(EditorContext *ctx, struct FileState *fs);
void go_to_line(EditorContext *ctx, struct FileState *fs, int line) __attribute__((weak));
__attribute__((weak)) int get_line_number_offset(struct FileState *fs);
void on_sigwinch(int sig);
void perform_resize(void);
void clamp_scroll_x(struct FileState *fs);
void cleanup_on_exit(struct FileManager *fm);
void disable_ctrl_c_z(void);
void apply_colors(void);

typedef struct {
    int x;
    int y;
} CursorPos;

CursorPos next_file(EditorContext *ctx);
CursorPos prev_file(EditorContext *ctx);
void handle_redo_wrapper(struct FileState *fs, int *cx, int *cy);
void handle_undo_wrapper(struct FileState *fs, int *cx, int *cy);
void allocation_failed(const char *msg);



#endif // EDITOR_H
