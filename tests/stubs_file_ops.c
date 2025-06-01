#include <ncurses.h>
#include "editor.h"
#include "files.h"
#include "file_manager.h"
#include "ui.h"

WINDOW *text_win = NULL;
FileState *active_file = NULL;
FileManager file_manager = {0};

void draw_text_buffer(FileState *fs, WINDOW *win){(void)fs;(void)win;}
void allocation_failed(const char *msg){(void)msg;}
void load_all_remaining_lines(FileState *fs){(void)fs;}
int load_next_lines(FileState *fs,int c){(void)fs;(void)c;return 0;}
FileState *initialize_file_state(const char *f,int m,int c){(void)f;(void)m;(void)c;return NULL;}
void free_file_state(FileState *f){(void)f;}
int fm_add(FileManager *fm, FileState *fs){(void)fm;(void)fs;return 0;}
void fm_close(FileManager *fm,int i){(void)fm;(void)i;}
FileState *fm_current(FileManager *fm){(void)fm;return NULL;}
int fm_switch(FileManager *fm,int i){(void)fm;(void)i;return 0;}
int show_open_file_dialog(char *p,int m){(void)p;(void)m;return 0;}
int show_save_file_dialog(char *p,int m){(void)p;(void)m;return 0;}
void update_status_bar(FileState *fs){(void)fs;}
void redraw(void){}
int show_message(const char *msg){(void)msg;return 0;}
bool any_file_modified(FileManager *fm){(void)fm;return false;}
