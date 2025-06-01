#ifndef SEARCH_H
#define SEARCH_H

#include "files.h"
void find_next_occurrence(struct FileState *fs, const char *word);
struct EditorContext;
void find(struct EditorContext *ctx, struct FileState *fs, int new_search);
void replace_next_occurrence(struct FileState *fs, const char *search,
                             const char *replacement);
void replace_all_occurrences(struct FileState *fs, const char *search,
                             const char *replacement);
void replace(struct EditorContext *ctx, struct FileState *fs);

#endif
