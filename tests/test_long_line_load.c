#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>
#include "files.h"

/* minimal WINDOW stubs */
WINDOW *newwin(int nlines,int ncols,int y,int x){(void)nlines;(void)ncols;(void)y;(void)x;return (WINDOW*)1;}
int delwin(WINDOW*w){(void)w;return 0;}
int keypad(WINDOW*w,bool b){(void)w;(void)b;return 0;}
int meta(WINDOW*w,bool b){(void)w;(void)b;return 0;}

int main(void){
    const char *fname = "tmp_long_line.txt";
    FILE *fp = fopen(fname, "w");
    char longline[1500];
    memset(longline, 'a', sizeof(longline)-1);
    longline[sizeof(longline)-1] = '\0';
    fprintf(fp, "%s\n", longline);
    fclose(fp);

    FileState *fs = initialize_file_state(fname, 5, 10);
    assert(fs);
    assert(load_file_into_buffer(fs) == 0);
    assert(fs->buffer.count == 1);
    assert(strlen(fs->buffer.lines[0]) == strlen(longline));
    assert(strcmp(fs->buffer.lines[0], longline) == 0);

    free_file_state(fs);
    unlink(fname);
    return 0;
}
