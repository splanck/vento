#include <ncurses.h>
#include <string.h>
#include <stdio.h>
#include "editor.h"
#include "ui.h"
#include "syntax.h"
#include "file_ops.h"
#include "files.h"
#include "file_manager.h"

void save_file(FileState *fs) {
    if (strlen(current_filename) == 0) {
        save_file_as(fs);
    } else {
        FILE *fp = fopen(current_filename, "w");
        if (fp) {
            for (int i = 0; i < fs->line_count; ++i) {
                fprintf(fp, "%s\n", fs->text_buffer[i]);
            }
            fclose(fp);
            mvprintw(LINES - 2, 2, "File saved as %s", current_filename);
        } else {
            mvprintw(LINES - 2, 2, "Error saving file!");
        }
        refresh();
        getch();
        mvprintw(LINES - 2, 2, "                            ");
        refresh();
    }
}

void save_file_as(FileState *fs) {
    create_dialog("Save as", current_filename, 256);

    FILE *fp = fopen(current_filename, "w");
    if (fp) {
        for (int i = 0; i < fs->line_count; ++i) {
            fprintf(fp, "%s\n", fs->text_buffer[i]);
        }
        fclose(fp);
        mvprintw(LINES - 2, 2, "File saved as %s", current_filename);
    } else {
        mvprintw(LINES - 2, 2, "Error saving file!");
    }

    refresh();
    getch();
    mvprintw(LINES - 2, 2, "                            ");
    refresh();
}

void load_file(FileState *fs_unused, const char *filename) {
    (void)fs_unused;
    char file_to_load[256];

    if (filename == NULL) {
        create_dialog("Load file", file_to_load, 256);
        filename = file_to_load;
    }

    /* Allocate a new file state */
    FileState *fs = initialize_file_state(filename, MAX_LINES, COLS - 3);
    active_file = fs;

    fs->syntax_mode = set_syntax_mode(filename);
    strcpy(fs->filename, filename);

    initialize_buffer();

    FILE *fp = fopen(filename, "r");
    if (fp) {
        fs->line_count = 0;
        while (fgets(fs->text_buffer[fs->line_count], COLS - 3, fp) && fs->line_count < MAX_LINES) {
            fs->text_buffer[fs->line_count][strcspn(fs->text_buffer[fs->line_count], "\n")] = '\0';
            fs->line_count++;
        }
        fclose(fp);
        mvprintw(LINES - 2, 2, "File loaded: %s", filename);

        fs->in_multiline_comment = false;
        fs->last_scanned_line = 0;
        fs->last_comment_state = false;

        strcpy(current_filename, filename);
    } else {
        mvprintw(LINES - 2, 2, "Error loading file!");
    }

    refresh();

    timeout(100);
    getch();
    timeout(-1);

    mvprintw(LINES - 2, 2, "                            ");
    refresh();
    text_win = fs->text_win;
    keypad(text_win, TRUE);
    meta(text_win, TRUE);

    box(text_win, 0, 0);
    wmove(text_win, 1, 1);

    draw_text_buffer(fs, text_win);
    wrefresh(text_win);

    int idx = fm_add(&file_manager, fs);
    fm_switch(&file_manager, idx);
    active_file = fm_current(&file_manager);

    strncpy(current_filename, active_file->filename, sizeof(current_filename) - 1);
    current_filename[sizeof(current_filename) - 1] = '\0';
    update_status_bar(active_file->cursor_y, active_file->cursor_x, active_file);
}

void new_file(FileState *fs_unused) {
    (void)fs_unused;
    FileState *fs = initialize_file_state("", MAX_LINES, COLS - 3);
    active_file = fs;
    strcpy(current_filename, "");
    fs->syntax_mode = set_syntax_mode(fs->filename);

    initialize_buffer();

    text_win = fs->text_win;
    keypad(text_win, TRUE);
    meta(text_win, TRUE);
    box(text_win, 0, 0);
    wmove(text_win, fs->cursor_y, fs->cursor_x);
    wrefresh(text_win);

    int idx = fm_add(&file_manager, fs);
    fm_switch(&file_manager, idx);
    active_file = fm_current(&file_manager);

    strncpy(current_filename, active_file->filename, sizeof(current_filename) - 1);
    current_filename[sizeof(current_filename) - 1] = '\0';
    update_status_bar(active_file->cursor_y, active_file->cursor_x, active_file);
}

void close_current_file(FileState *fs_unused, int *cx, int *cy) {
    (void)fs_unused;
    fm_close(&file_manager, file_manager.active_index);

    if (file_manager.count > 0) {
        active_file = fm_current(&file_manager);
        text_win = active_file->text_win;
        strncpy(current_filename, active_file->filename, sizeof(current_filename) - 1);
        current_filename[sizeof(current_filename) - 1] = '\0';
    } else {
        new_file(NULL);
    }

    active_file = fm_current(&file_manager);
    if (cx && cy && active_file) {
        *cx = active_file->cursor_x;
        *cy = active_file->cursor_y;
    }
    redraw(cx, cy);
    update_status_bar(active_file->cursor_y, active_file->cursor_x, active_file);
}

int set_syntax_mode(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (ext) {
        if (strcmp(ext, ".c") == 0 || strcmp(ext, ".h") == 0) {
            return C_SYNTAX;
        } else if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) {
            return HTML_SYNTAX;
        } else if (strcmp(ext, ".py") == 0) {
            return PYTHON_SYNTAX;
        } else if (strcmp(ext, ".cs") == 0) {
            return CSHARP_SYNTAX;
        }
    }
    return NO_SYNTAX;
}


