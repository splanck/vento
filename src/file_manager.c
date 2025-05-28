#include <stdlib.h>
#include "file_manager.h"

FileManager file_manager;

void fm_init(FileManager *fm) {
    fm->files = NULL;
    fm->count = 0;
    fm->active_index = -1;
}

FileState *fm_current(FileManager *fm) {
    if (!fm || fm->active_index < 0 || fm->active_index >= fm->count) {
        return NULL;
    }
    return fm->files[fm->active_index];
}

int fm_add(FileManager *fm, FileState *fs) {
    if (!fm || !fs) return -1;
    FileState **tmp = realloc(fm->files, sizeof(FileState*) * (fm->count + 1));
    if (!tmp) return -1;
    fm->files = tmp;
    fm->files[fm->count] = fs;
    fm->active_index = fm->count;
    fm->count++;
    return fm->active_index;
}

void fm_close(FileManager *fm, int index) {
    if (!fm || index < 0 || index >= fm->count) return;
    free_file_state(fm->files[index], MAX_LINES);
    for (int i = index; i < fm->count - 1; i++) {
        fm->files[i] = fm->files[i + 1];
    }
    fm->count--;
    if (fm->count == 0) {
        free(fm->files);
        fm->files = NULL;
        fm->active_index = -1;
    } else {
        fm->active_index = (index <= fm->active_index && fm->active_index > 0) ? fm->active_index - 1 : fm->active_index;
        if (fm->active_index >= fm->count) fm->active_index = fm->count - 1;
    }
}

int fm_switch(FileManager *fm, int index) {
    if (!fm || index < 0 || index >= fm->count) return -1;
    fm->active_index = index;
    return fm->active_index;
}

