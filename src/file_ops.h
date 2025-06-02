#ifndef FILE_OPS_H
#define FILE_OPS_H

struct FileState;
struct EditorContext;
#include <stdbool.h>
void save_file(struct EditorContext *ctx, struct FileState *fs);
void save_file_as(struct EditorContext *ctx, struct FileState *fs);
int load_file(struct EditorContext *ctx, struct FileState *fs,
              const char *filename);
void new_file(struct EditorContext *ctx, struct FileState *fs);
void close_current_file(struct EditorContext *ctx, struct FileState *fs,
                        int *cx, int *cy);
int set_syntax_mode(const char *filename);
bool confirm_switch(void);

#endif // FILE_OPS_H
