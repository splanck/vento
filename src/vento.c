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


/**
 * The main function of the program.
 * 
 * @param argc The number of command-line arguments.
 * @param argv An array of strings containing the command-line arguments.
 * @return 0 on successful execution.
 */
int main(int argc, char *argv[]) {
    initialize();

    fm_init(&file_manager);

    // Load initial file or create a new one
    if (argc > 1) {
        load_file(NULL, argv[1]);
    } else {
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
