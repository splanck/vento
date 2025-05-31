#include <ncurses.h>
#include "config.h"
#undef wrefresh
#undef touchwin

WINDOW *stdscr = (WINDOW*)1;
int enable_color = 0;
int wrefresh(WINDOW*w){(void)w;return 0;}
int touchwin(WINDOW*w){(void)w;return 0;}
int enable_mouse = 0;
int show_line_numbers = 0;
AppConfig app_config = {0};

WINDOW *create_popup_window(int h, int w, WINDOW *parent){(void)h;(void)w;(void)parent;return (WINDOW*)1;}
int show_message(const char*msg){(void)msg;return 0;}
short get_color_code(const char*name){(void)name;return 0;}
void load_theme(const char*name, AppConfig*cfg){(void)name;(void)cfg;}


