#include <assert.h>
#include <stdlib.h>
#include <ncurses.h>
#include "file_manager.h"
#include "files.h"

int main(void){
    FileManager fm;
    fm_init(&fm);

    FileState *states[10];
    for(int i=0;i<10;i++){
        states[i] = calloc(1, sizeof(FileState));
        assert(states[i]);
        fm_add(&fm, states[i]);
        assert(fm.count == i + 1);
        assert(fm.capacity == fm.count);
    }

    for(int i=9;i>=0;i--){
        fm_close(&fm, i);
        assert(fm.count == i);
        assert(fm.capacity == fm.count);
    }
    return 0;
}
