#ifndef SEARCH_H
#define SEARCH_H

#include "files.h"
void find_next_occurrence(struct FileState *fs, const char *word);
void find(struct FileState *fs, int new_search);

#endif
