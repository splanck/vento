#include <ncurses.h>
#include <string.h>
#include <stdio.h>
#include "editor.h"
#include "ui.h"
#include "syntax.h"
#include "file_ops.h"

void save_file() {
    if (strlen(current_filename) == 0) {
        save_file_as();
    } else {
        FILE *fp = fopen(current_filename, "w");
        if (fp) {
            for (int i = 0; i < line_count; ++i) {
                fprintf(fp, "%s\n", text_buffer[i]);
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

void save_file_as() {
    create_dialog("Save as", current_filename, 256);

    FILE *fp = fopen(current_filename, "w");
    if (fp) {
        for (int i = 0; i < line_count; ++i) {
            fprintf(fp, "%s\n", text_buffer[i]);
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

void load_file(const char *filename) {
    char file_to_load[256];

    if (filename == NULL) {
        create_dialog("Load file", file_to_load, 256);
        filename = file_to_load;
    }

    set_syntax_mode(filename);
    initialize_buffer();

    FILE *fp = fopen(filename, "r");
    if (fp) {
        line_count = 0;
        while (fgets(text_buffer[line_count], COLS - 3, fp) && line_count < MAX_LINES) {
            text_buffer[line_count][strcspn(text_buffer[line_count], "\n")] = '\0';
            line_count++;
        }
        fclose(fp);
        mvprintw(LINES - 2, 2, "File loaded: %s", filename);

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
    text_win = newwin(LINES - 2, COLS, 1, 0);
    keypad(text_win, TRUE);
    meta(text_win, TRUE);

    box(text_win, 0, 0);
    wmove(text_win, 1, 1);

    draw_text_buffer(text_win);
    wrefresh(text_win);
}

void new_file() {
    int cursor_x = 1, cursor_y = 1;
    strcpy(current_filename, "");

    initialize_buffer();

    text_win = newwin(LINES - 2, COLS, 1, 0);
    keypad(text_win, TRUE);
    meta(text_win, TRUE);
    box(text_win, 0, 0);
    wmove(text_win, cursor_y, cursor_x);
    wrefresh(text_win);
}

void set_syntax_mode(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (ext) {
        if (strcmp(ext, ".c") == 0 || strcmp(ext, ".h") == 0) {
            current_syntax_mode = C_SYNTAX;
        } else if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) {
            current_syntax_mode = HTML_SYNTAX;
        } else if (strcmp(ext, ".py") == 0) {
            current_syntax_mode = PYTHON_SYNTAX;
        } else if (strcmp(ext, ".cs") == 0) {
            current_syntax_mode = CSHARP_SYNTAX;
        } else {
            current_syntax_mode = NO_SYNTAX;
        }
    } else {
        current_syntax_mode = NO_SYNTAX;
    }
}


