#ifndef SEARCH_H
#define SEARCH_H

#include "files.h"
void find_next_occurrence(struct FileState *fs, const char *word);
void find(struct FileState *fs, int new_search);
void replace_next_occurrence(struct FileState *fs, const char *search,
                             const char *replacement);
void replace_all_occurrences(struct FileState *fs, const char *search,
                             const char *replacement);
void replace(struct FileState *fs);

#endif
