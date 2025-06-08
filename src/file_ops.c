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
#include "path_utils.h"
#include <stdlib.h>
#include <errno.h>

/*
 * file_ops.c
 * ----------
 * High level helpers for creating new buffers, loading and saving files and
 * switching which buffer is active.  These routines interact with the global
 * FileManager to keep the list of open files in sync and perform lazy loading
 * of file contents.  Because they are triggered from the UI layer they also
 * refresh editor windows and the status bar when needed.
 */

#define INITIAL_LOAD_LINES 1024

/*
 * Save the current buffer to the file referenced by `fs`.
 *
 *  ctx - Optional EditorContext.  Currently unused but passed for API symmetry.
 *  fs  - FileState describing the buffer to write.
 *
 * Before writing the buffer the function calls load_all_remaining_lines so that
 * a lazily loaded file is fully realized on disk.  On success the file is
 * truncated and rewritten line by line, `fs->modified` is cleared and a brief
 * status message is shown.  Errors simply display a message; the undo history
 * is unaffected and the caller must handle further recovery.  No redraw occurs
 * aside from the status bar updates.
 */
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
            int err = errno;
            mvprintw(LINES - 2, 2, "Error saving file: %s", strerror(err));
        }
        clrtoeol();
        refresh();
        napms(1000);      /* display for one second */
        mvprintw(LINES - 2, 2, "%*s", COLS - 4, "");
        refresh();
        return;
    }
}

/*
 * Prompt the user for a new filename and save the buffer there.
 *
 *  ctx - Editor context used for presenting the save-as dialog.
 *  fs  - FileState whose buffer should be written.
 *
 * The chosen path is canonicalized and stored back into `fs->filename` before
 * writing.  Like save_file() this ensures any lazily loaded portions are read
 * first.  On success the modified flag is cleared and a short message is
 * displayed.  Failure simply reports an error.  No undo information changes and
 * only the status bar is redrawn.
 */
void save_file_as(EditorContext *ctx, FileState *fs) {
    (void)ctx;
    char newpath[256];
    if (!show_save_file_dialog(ctx, newpath, sizeof(newpath)))
        return;    // user cancelled
    canonicalize_path(newpath, fs->filename, sizeof(fs->filename));

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
        int err = errno;
        mvprintw(LINES - 2, 2, "Error saving file: %s", strerror(err));
    }

    clrtoeol();
    refresh();
    napms(1000);      /* display for one second */
    mvprintw(LINES - 2, 2, "%*s", COLS - 4, "");
    refresh();
    return;
}

/*
 * Load the file specified by `filename` into a new FileState and make it the
 * active file.  If `filename` is NULL an open-file dialog is displayed.
 *
 *  ctx        - Editor context for updating global state and UI.
 *  fs_unused  - Unused parameter required by the command handler prototype.
 *  filename   - Path to the file to open or NULL to prompt the user.
 *
 * Only the first INITIAL_LOAD_LINES lines are read immediately to keep large
 * files responsive.  The new FileState is inserted into the FileManager and the
 * previous active file is detached, closing its stream if it was only partially
 * loaded.  The associated ncurses window is drawn and refreshed.  On error a
 * message is displayed and the previous file remains active.
 *
 * Returns 0 on success or -1 on failure or user cancellation.  Undo history for
 * the new file starts empty.
 */
int load_file(EditorContext *ctx, FileState *fs_unused, const char *filename) {
    (void)fs_unused;
    char file_to_load[256];
    char canonical[PATH_MAX];
    FileState *previous_active = active_file;

    if (filename == NULL) {
        if (show_open_file_dialog(ctx, file_to_load, sizeof(file_to_load)) == 0) {
            sync_editor_context(ctx);
            return -1; // user cancelled
        }
        filename = file_to_load;
    }

    const char *filename_canon = filename;
    if (filename) {
        canonicalize_path(filename, canonical, sizeof(canonical));
        filename_canon = canonical;
    }

    /* If the file is already open, just switch to it */
    for (int i = 0; i < file_manager.count; i++) {
        FileState *open_fs = file_manager.files[i];
        if (open_fs && strcmp(open_fs->filename, filename_canon) == 0) {
            fm_switch(&file_manager, i);
            active_file = open_fs;
            if (previous_active && previous_active != active_file &&
                previous_active->fp && !previous_active->file_complete) {
                /*
                 * The previous file was only partially loaded. Store the current
                 * position and close the handle so it can be re-opened lazily
                 * later without keeping the descriptor open.
                 */
                previous_active->file_pos = ftell(previous_active->fp);
                fclose(previous_active->fp);
                previous_active->fp = NULL;
            }
            text_win = open_fs->text_win;
            if (ctx) {
                ctx->file_manager = file_manager;
                ctx->active_file = active_file;
                ctx->text_win = text_win;
                sync_editor_context(ctx);
            }
            update_status_bar(ctx, active_file);
            sync_editor_context(ctx);
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
    canonicalize_path(filename_canon, fs->filename, sizeof(fs->filename));


    fs->fp = fopen(filename_canon, "r");
    fs->file_pos = 0;
    if (!fs->fp) {
        int err = errno;
        mvprintw(LINES - 2, 2, "Error loading file: %s", strerror(err));
        refresh();
        getch();
        mvprintw(LINES - 2, 2, "                            ");
        refresh();
        free_file_state(fs);
        active_file = previous_active;
        text_win = previous_active ? previous_active->text_win : NULL;
        sync_editor_context(ctx);
        return -1;
    }
    fs->file_complete = false;
    fs->buffer.count = 0;
    if (load_next_lines(fs, INITIAL_LOAD_LINES) < 0) {
        int err = errno;
        mvprintw(LINES - 2, 2, "Error loading file: %s", strerror(err));
        refresh();
        getch();
        mvprintw(LINES - 2, 2, "                            ");
        refresh();
        free_file_state(fs);
        active_file = previous_active;
        text_win = previous_active ? previous_active->text_win : NULL;
        sync_editor_context(ctx);
        return -1;
    }
    mvprintw(LINES - 2, 2, "File loaded: %s", filename_canon);

    fs->in_multiline_comment = false;
    fs->last_scanned_line = 0;
    fs->last_comment_state = false;
    fs->modified = false;

    canonicalize_path(filename_canon, fs->filename, sizeof(fs->filename));

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
        int err = errno;
        mvprintw(LINES - 2, 2, "Error loading file: %s", strerror(err));
        refresh();
        getch();
        mvprintw(LINES - 2, 2, "                            ");
        refresh();
        free_file_state(fs);
        active_file = previous_active;
        text_win = previous_active ? previous_active->text_win : NULL;
        sync_editor_context(ctx);
        return -1;
    }

    if (file_manager.count > 1) {
        FileState *first = file_manager.files[0];
        if (first && first->filename[0] == '\0' && !first->modified) {
            fm_close(&file_manager, 0);
            sync_editor_context(ctx);
            idx = file_manager.active_index;
        }
    }

    fm_switch(&file_manager, idx);
    active_file = fm_current(&file_manager);
    if (previous_active && previous_active != active_file &&
        previous_active->fp && !previous_active->file_complete) {
        /*
         * Record the offset of the partially loaded file and close its stream
         * so that it may be reopened on demand without consuming resources.
         */
        previous_active->file_pos = ftell(previous_active->fp);
        fclose(previous_active->fp);
        previous_active->fp = NULL;
    }

    ctx->file_manager = file_manager;
    ctx->active_file = active_file;
    ctx->text_win = text_win;
    sync_editor_context(ctx);

    update_status_bar(ctx, active_file);
    extern int start_line;
    if (start_line > 0 && go_to_line)
        go_to_line(ctx, active_file, start_line);
    start_line = 0;    /* only apply once */
    sync_editor_context(ctx);
    return 0;
}

/*
 * Create a new empty buffer and make it the active file.
 *
 *  ctx       - Editor context to update after switching files.
 *  fs_unused - Present to conform to command handler prototypes.
 *
 * A fresh FileState with an empty undo/redo history is allocated and inserted
 * into the FileManager.  If activation succeeds the new buffer becomes current
 * and its ncurses window is boxed and refreshed.  On failure the previous file
 * remains active.  The status bar is updated but no full redraw is triggered.
 */
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
        if (ctx)
            sync_editor_context(ctx);
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
        if (ctx)
            sync_editor_context(ctx);
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

    if (ctx)
        sync_editor_context(ctx);
    update_status_bar(ctx, active_file);
}

/*
 * Close the file currently selected in the FileManager.
 *
 *  ctx       - Editor context used for UI updates.
 *  fs_unused - Present for command handler compatibility.
 *  cx, cy    - Optional pointers that receive the cursor position of the file
 *              that becomes active after the close.
 *
 * If the buffer is modified the user is prompted to save.  The FileManager
 * entry is removed and another file is activated or a fresh one created when no
 * files remain.  The caller's cursor pointers are updated accordingly and the
 * entire screen is redrawn.  Undo stacks are left intact for remaining files.
 */
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
    sync_editor_context(ctx);

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

/*
 * Ask the user whether it is okay to switch files when unsaved modifications
 * exist in any open buffer.
 *
 * The check scans the FileManager for modified buffers and prompts the user if
 * any are found.  No state changes are made here; a simple yes/no response is
 * returned for the caller to act upon.
 */
bool confirm_switch(void) {
    if (!any_file_modified(&file_manager))
        return true;

    int ch = show_message("Unsaved changes. Switch files? (y/n)");
    if (ch == 'y' || ch == 'Y')
        return true;
    if (ch == 'n' || ch == 'N')
        return false;
    return false;
}

/*
 * Determine the syntax highlighting mode based on the filename or file
 * contents.  The extension is checked first; if it is missing the function
 * attempts to detect a shebang line for scripting languages.
 */
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


