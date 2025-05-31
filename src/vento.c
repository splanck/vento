#include <ncurses.h>
#include <stdlib.h>
#include <signal.h>
#include "editor.h"
#include "file_ops.h"
#include "undo.h"
#include "input.h"
#include "ui.h"
#include "files.h"
#include "file_manager.h"
#include "ui_common.h"
#include <stdbool.h>

bool confirm_quit(void) {
    if (!any_file_modified(&file_manager))
        return true;

    int ch = show_message("Unsaved changes. Quit anyway? (y/n)");
    if (ch == 'y' || ch == 'Y')
        return true;
    if (ch == 'n' || ch == 'N')
        return false;
    return false;
}


/**
 * The main function of the program.
 * 
 * @param argc The number of command-line arguments.
 * @param argv An array of strings containing the command-line arguments.
 * @return 0 on successful execution.
 */
int main(int argc, char *argv[]) {
    int file_count = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Usage: %s [options] [file...]\n", argv[0]);
            printf("  -h, --help     Show this help and exit\n");
            printf("  -v, --version  Print version information and exit\n");
            return 0;
        } else if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0) {
            printf("%s\n", VERSION);
            return 0;
        }
    }

    initialize();

    fm_init(&file_manager);

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0 ||
            strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0) {
            continue;
        }
        load_file(NULL, argv[i]);
        file_count++;
    }

    if (file_count == 0) {
        new_file(NULL);
    }

    active_file = fm_current(&file_manager);

    // Show the warning dialog before entering the main editor loop
    show_warning_dialog();

    // Finishes initializing editor window and begin accepting keyboard input.
    run_editor();
    
    endwin();

    cleanup_on_exit(&file_manager);

    active_file = NULL;

    return 0;
}
