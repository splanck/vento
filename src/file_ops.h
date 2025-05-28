#ifndef FILE_OPS_H
#define FILE_OPS_H

struct FileState;
void save_file(struct FileState *fs);
void save_file_as(struct FileState *fs);
void load_file(struct FileState *fs, const char *filename);
void new_file(struct FileState *fs);
void close_current_file(struct FileState *fs, int *cx, int *cy);
int set_syntax_mode(const char *filename);

#endif // FILE_OPS_H
