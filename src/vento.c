#include <ncurses.h>
#include <stdlib.h>
#include <signal.h>
#include "editor.h"

int main(int argc, char *argv[]) {
    initialize();

    initialize_buffer();

    // Check if a filename was provided as a parameter
    if (argc > 1) {
        load_file(argv[1]);
    } else {
        new_file();
    }

    run_editor();
    endwin();
    return 0;
}
