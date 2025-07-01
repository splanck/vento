
/*
 * file_manager.c
 * --------------
 * Implements the FileManager used by the editor.  A FileManager holds an
 * array of FileState pointers representing every open file.  The index of the
 * currently active file is stored in `active_index` and `capacity` tracks the
 * size of the allocated array.  The global instance is initialised in main()
 * via fm_init before any files are opened.
 */

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "file_manager.h"
#include "editor_state.h"

/*
 * Initialize a FileManager to an empty state.
 *
 * fm - Pointer to the FileManager to set up.
 *
 * The file list is cleared, active_index is set to -1 and no memory is
 * allocated for file storage.
 */
void fm_init(FileManager *fm) {
    fm->files = NULL;
    fm->count = 0;
    fm->active_index = -1;
    fm->capacity = 0;
}

/*
 * Get the FileState currently marked as active.
 *
 * fm - FileManager containing the list of files.
 *
 * Returns the active FileState pointer or NULL if no file is selected.
 */
FileState *fm_current(FileManager *fm) {
    if (!fm || fm->active_index < 0 || fm->active_index >= fm->count) {
        return NULL;
    }
    return fm->files[fm->active_index];
}

/*
 * Add a new file to the FileManager and make it active.
 *
 * fm - FileManager managing open files.
 * fs - Newly created FileState to insert.
 *
 * Returns the index of the inserted file or -1 on allocation failure.
 * The file list may be reallocated and `active_index` will point to the
 * newly added entry.
 */
int fm_add(FileManager *fm, FileState *fs) {
    if (!fm || !fs) return -1;
    FileState **tmp = realloc(fm->files, sizeof(FileState*) * (fm->count + 1));
    if (!tmp) return -1;
    fm->files = tmp;
    fm->files[fm->count] = fs;
    fm->active_index = fm->count;
    fm->capacity = fm->count + 1;
    fm->count++;
    return fm->active_index;
}

/*
 * Remove a file from the FileManager.
 *
 * fm    - FileManager managing the list of files.
 * index - Index of the file to close.
 *
 * If the file was only partially loaded, its current position is stored and
 * the FILE pointer closed before freeing the FileState.  The entry is removed
 * from the array, memory is freed and `active_index`, `count` and `capacity`
 * are updated accordingly.
 */
void fm_close(FileManager *fm, int index) {
    if (!fm || index < 0 || index >= fm->count) return;
    FileState *fs = fm->files[index];
    if (fs && fs->fp && !fs->file_complete) {
        fs->file_pos = ftell(fs->fp);
        fclose(fs->fp);
        fs->fp = NULL;
    }
    free_file_state(fs);
    for (int i = index; i < fm->count - 1; i++) {
        fm->files[i] = fm->files[i + 1];
    }
    fm->count--;
    if (fm->count == 0) {
        free(fm->files);
        fm->files = NULL;
        fm->active_index = -1;
        fm->capacity = 0;
    } else {
        FileState **tmp = realloc(fm->files, sizeof(FileState*) * fm->count);
        if (tmp) {
            fm->files = tmp;
            fm->capacity = fm->count;
        }
        fm->active_index = (index <= fm->active_index && fm->active_index > 0) ? fm->active_index - 1 : fm->active_index;
        if (fm->active_index >= fm->count) fm->active_index = fm->count - 1;
    }
}

/*
 * Switch the currently active file.
 *
 * fm    - FileManager managing the list of files.
 * index - Index of the file to make active.
 *
 * Returns the new active index or -1 if the index is invalid.  Only the
 * active_index field is changed.
 */
int fm_switch(FileManager *fm, int index) {
    if (!fm || index < 0 || index >= fm->count) return -1;
    fm->active_index = index;
    return fm->active_index;
}

/*
 * Determine if any tracked file has unsaved modifications.
 *
 * fm - FileManager to inspect.
 *
 * Returns true if any FileState in the list has the `modified` flag set.
 */
bool any_file_modified(FileManager *fm) {
    if (!fm) return false;
    for (int i = 0; i < fm->count; i++) {
        FileState *fs = fm->files[i];
        if (fs && fs->modified)
            return true;
    }
    return false;
}

