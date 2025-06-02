#include <ncurses.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "editor.h"
#include "ui.h"
#include "syntax.h"
#include "file_ops.h"
#include "files.h"
#include "file_manager.h"
#include "ui_common.h"
#include "editor_state.h"
#include <limits.h>
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
#include <stdlib.h>
char *realpath(const char *path, char *resolved_path);

#define INITIAL_LOAD_LINES 1024

void save_file(EditorContext *ctx, FileState *fs) {
    (void)ctx;
    if (strlen(fs->filename) == 0) {
        save_file_as(ctx, fs);
    } else {
        load_all_remaining_lines(fs);
        FILE *fp = fopen(fs->filename, "w");
        if (fp) {
            for (int i = 0; i < fs->buffer.count; ++i) {
                const char *ln = lb_get(&fs->buffer, i);
                if (!ln)
                    ln = "";
                fprintf(fp, "%s\n", ln);
            }
            fclose(fp);
            fs->modified = false;
            mvprintw(LINES - 2, 2, "File saved as %s", fs->filename);
        } else {
            mvprintw(LINES - 2, 2, "Error saving file!");
        }
        clrtoeol();
        refresh();
        napms(1000);      /* display for one second */
        mvprintw(LINES - 2, 2, "%*s", COLS - 4, "");
        refresh();
        return;
    }
}

void save_file_as(EditorContext *ctx, FileState *fs) {
    (void)ctx;
    char newpath[256];
    if (!show_save_file_dialog(ctx, newpath, sizeof(newpath)))
        return;    // user cancelled
    strncpy(fs->filename, newpath, sizeof(fs->filename) - 1);
    fs->filename[sizeof(fs->filename) - 1] = '\0';

    load_all_remaining_lines(fs);

    FILE *fp = fopen(fs->filename, "w");
    if (fp) {
        for (int i = 0; i < fs->buffer.count; ++i) {
            const char *ln = lb_get(&fs->buffer, i);
            if (!ln)
                ln = "";
            fprintf(fp, "%s\n", ln);
        }
        fclose(fp);
        fs->modified = false;
        mvprintw(LINES - 2, 2, "File saved as %s", fs->filename);
    } else {
        mvprintw(LINES - 2, 2, "Error saving file!");
    }

    clrtoeol();
    refresh();
    napms(1000);      /* display for one second */
    mvprintw(LINES - 2, 2, "%*s", COLS - 4, "");
    refresh();
    return;
}

int load_file(EditorContext *ctx, FileState *fs_unused, const char *filename) {
    (void)fs_unused;
    char file_to_load[256];
    char canonical[PATH_MAX];
    FileState *previous_active = active_file;

    if (filename == NULL) {
        if (show_open_file_dialog(ctx, file_to_load, sizeof(file_to_load)) == 0) {
            return -1; // user cancelled
        }
        filename = file_to_load;
    }

    const char *filename_canon = filename;
    if (filename && realpath(filename, canonical))
        filename_canon = canonical;

    /* If the file is already open, just switch to it */
    for (int i = 0; i < file_manager.count; i++) {
        FileState *open_fs = file_manager.files[i];
        if (open_fs && strcmp(open_fs->filename, filename_canon) == 0) {
            fm_switch(&file_manager, i);
            active_file = open_fs;
            text_win = open_fs->text_win;
            if (ctx) {
                ctx->file_manager = file_manager;
                ctx->active_file = active_file;
                ctx->text_win = text_win;
            }
            update_status_bar(ctx, active_file);
            return 0;
        }
    }

    /* Allocate a new file state */
    FileState *fs = initialize_file_state(filename_canon, DEFAULT_BUFFER_LINES, COLS - 3);
    if (!fs) {
        allocation_failed("initialize_file_state failed");
    }
    active_file = fs;

    fs->syntax_mode = set_syntax_mode(filename_canon);
    strncpy(fs->filename, filename_canon, sizeof(fs->filename) - 1);
    fs->filename[sizeof(fs->filename) - 1] = '\0';


    fs->fp = fopen(filename_canon, "r");
    fs->file_pos = 0;
    if (!fs->fp) {
        mvprintw(LINES - 2, 2, "Error loading file!");
        refresh();
        getch();
        mvprintw(LINES - 2, 2, "                            ");
        refresh();
        free_file_state(fs);
        active_file = previous_active;
        text_win = previous_active ? previous_active->text_win : NULL;
        return -1;
    }
    fs->file_complete = false;
    fs->buffer.count = 0;
    if (load_next_lines(fs, INITIAL_LOAD_LINES) < 0) {
        mvprintw(LINES - 2, 2, "Error loading file!");
        refresh();
        getch();
        mvprintw(LINES - 2, 2, "                            ");
        refresh();
        free_file_state(fs);
        active_file = previous_active;
        text_win = previous_active ? previous_active->text_win : NULL;
        return -1;
    }
    mvprintw(LINES - 2, 2, "File loaded: %s", filename_canon);

    fs->in_multiline_comment = false;
    fs->last_scanned_line = 0;
    fs->last_comment_state = false;
    fs->modified = false;

    strncpy(fs->filename, filename_canon, sizeof(fs->filename) - 1);
    fs->filename[sizeof(fs->filename) - 1] = '\0';

    refresh();

    timeout(100);
    getch();
    timeout(-1);

    mvprintw(LINES - 2, 2, "                            ");
    refresh();
    text_win = fs->text_win;

    box(text_win, 0, 0);
    wmove(text_win, 1, 1 + get_line_number_offset(fs));

    draw_text_buffer(fs, text_win);
    fs->cursor_x = fs->saved_cursor_x;
    fs->cursor_y = fs->saved_cursor_y;
    wmove(text_win, fs->cursor_y,
          fs->cursor_x + get_line_number_offset(fs));
    wrefresh(text_win);

    int idx = fm_add(&file_manager, fs);
    if (idx < 0) {
        mvprintw(LINES - 2, 2, "Error loading file!");
        refresh();
        getch();
        mvprintw(LINES - 2, 2, "                            ");
        refresh();
        free_file_state(fs);
        active_file = previous_active;
        text_win = previous_active ? previous_active->text_win : NULL;
        return -1;
    }
    fm_switch(&file_manager, idx);
    active_file = fm_current(&file_manager);

    ctx->file_manager = file_manager;
    ctx->active_file = active_file;
    ctx->text_win = text_win;

    update_status_bar(ctx, active_file);
    extern int start_line;
    if (start_line > 0 && go_to_line)
        go_to_line(ctx, active_file, start_line);
    start_line = 0;    /* only apply once */
    return 0;
}

void new_file(EditorContext *ctx, FileState *fs_unused) {
    (void)fs_unused;
    FileState *previous_active = active_file;
    int previous_index = file_manager.active_index;

    FileState *fs = initialize_file_state("", DEFAULT_BUFFER_LINES, COLS - 3);
    if (!fs) {
        allocation_failed("initialize_file_state failed");
    }

    fs->filename[0] = '\0';
    fs->syntax_mode = set_syntax_mode(fs->filename);
    fs->modified = false;

    int idx = fm_add(&file_manager, fs);
    if (idx < 0) {
        mvprintw(LINES - 2, 2, "Error creating file!");
        refresh();
        getch();
        mvprintw(LINES - 2, 2, "                            ");
        refresh();
        free_file_state(fs);
        file_manager.active_index = previous_index;
        active_file = previous_active;
        text_win = previous_active ? previous_active->text_win : NULL;
        return;
    }

    if (fm_switch(&file_manager, idx) < 0) {
        mvprintw(LINES - 2, 2, "Error activating file!");
        refresh();
        getch();
        mvprintw(LINES - 2, 2, "                            ");
        refresh();
        fm_close(&file_manager, idx);
        file_manager.active_index = previous_index;
        active_file = previous_active;
        text_win = previous_active ? previous_active->text_win : NULL;
        return;
    }

    active_file = fm_current(&file_manager);
    text_win = fs->text_win;
    box(text_win, 0, 0);
    fs->cursor_x = fs->saved_cursor_x;
    fs->cursor_y = fs->saved_cursor_y;
    wmove(text_win, fs->cursor_y,
          fs->cursor_x + get_line_number_offset(fs));
    wrefresh(text_win);

    update_status_bar(ctx, active_file);
    if (ctx)
        sync_editor_context(ctx);
}

void close_current_file(EditorContext *ctx, FileState *fs_unused, int *cx, int *cy) {
    (void)fs_unused;
    FileState *current = fm_current(&file_manager);
    int *orig_cx = cx;
    int *orig_cy = cy;
    bool cx_in_current = false;
    bool cy_in_current = false;

    if (current) {
        if (cx)
            cx_in_current = (cx == &current->cursor_x);
        if (cy)
            cy_in_current = (cy == &current->cursor_y);
        current->saved_cursor_x = current->cursor_x;
        current->saved_cursor_y = current->cursor_y;
    }
    if (current && current->modified) {
        int ch = show_message("File modified. Save before closing? (y/n)");
        if (ch == 'y' || ch == 'Y') {
            save_file(ctx, current);
        } else if (ch != 'n' && ch != 'N') {
            return; /* cancel on other keys */
        }
    }
    fm_close(&file_manager, file_manager.active_index);

    if (file_manager.count > 0) {
        active_file = fm_current(&file_manager);
        text_win = active_file->text_win;
    } else {
        new_file(ctx, NULL);
        /* new_file already updated ctx fields, but we'll refresh them below */
    }

    active_file = fm_current(&file_manager);
    if (active_file) {
        active_file->cursor_x = active_file->saved_cursor_x;
        active_file->cursor_y = active_file->saved_cursor_y;
        if (orig_cx && !cx_in_current)
            *orig_cx = active_file->cursor_x;
        if (orig_cy && !cy_in_current)
            *orig_cy = active_file->cursor_y;
    }
    if (ctx)
        sync_editor_context(ctx);
    redraw();
    update_status_bar(ctx, active_file);
}

bool confirm_switch(void) {
    if (!any_file_modified(&file_manager))
        return true;

    int ch = show_message("Unsaved changes. Switch files?");
    if (ch == 'y' || ch == 'Y')
        return true;
    if (ch == 'n' || ch == 'N')
        return false;
    return false;
}

int set_syntax_mode(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (ext && ext[1] != '\0') {
        if (strcmp(ext, ".c") == 0 || strcmp(ext, ".h") == 0) {
            return C_SYNTAX;
        } else if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) {
            return HTML_SYNTAX;
        } else if (strcmp(ext, ".py") == 0) {
            return PYTHON_SYNTAX;
        } else if (strcmp(ext, ".cs") == 0) {
            return CSHARP_SYNTAX;
        } else if (strcmp(ext, ".js") == 0) {
            return JS_SYNTAX;
        } else if (strcmp(ext, ".css") == 0) {
            return CSS_SYNTAX;
        }
    }
    FILE *fp = fopen(filename, "r");
    if (fp) {
        char first[256];
        if (fgets(first, sizeof(first), fp)) {
            char lower[256];
            size_t i;
            for (i = 0; i < sizeof(lower) - 1 && first[i]; i++) {
                lower[i] = (char)tolower((unsigned char)first[i]);
            }
            lower[i] = '\0';
            if (strncmp(lower, "#!", 2) == 0) {
                if (strstr(lower, "python")) {
                    fclose(fp);
                    return PYTHON_SYNTAX;
                }
                if (strstr(lower, "bash") || strstr(lower, "sh")) {
                    fclose(fp);
                    return SHELL_SYNTAX;
                }
            }
        }
        fclose(fp);
    }
    return NO_SYNTAX;
}


