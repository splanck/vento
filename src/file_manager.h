#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "files.h"
#include <stdbool.h>

typedef struct FileManager {
    FileState **files;
    int count;
    int active_index;
} FileManager;

extern FileManager file_manager;

void fm_init(FileManager *fm);
FileState *fm_current(FileManager *fm);
int fm_add(FileManager *fm, FileState *fs);
void fm_close(FileManager *fm, int index);
int fm_switch(FileManager *fm, int index);
bool any_file_modified(FileManager *fm);

#endif // FILE_MANAGER_H
