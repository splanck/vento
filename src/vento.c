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
#include "config.h"
#include "editor_state.h"
#include <stdbool.h>

extern void load_theme(const char *name, AppConfig *cfg) __attribute__((weak));
extern void apply_colors(void) __attribute__((weak));

static EditorContext editor;

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
    const char *theme_name = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Usage: %s [options] [file...]\n", argv[0]);
            printf("  -h, --help     Show this help and exit\n");
            printf("  -v, --version  Print version information and exit\n");
            printf("  -t, --theme    Load a color theme before opening files\n");
            return 0;
        } else if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0) {
            printf("%s\n", VERSION);
            return 0;
        } else if (strncmp(argv[i], "--theme=", 8) == 0) {
            theme_name = argv[i] + 8;
        } else if ((strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--theme") == 0) && i + 1 < argc) {
            theme_name = argv[++i];
        }
    }

    initialize(&editor);

    if (theme_name && *theme_name && load_theme) {
        load_theme(theme_name, &app_config);
        strncpy(app_config.theme, theme_name, sizeof(app_config.theme) - 1);
        app_config.theme[sizeof(app_config.theme) - 1] = '\0';
        if (apply_colors)
            apply_colors();
    }

    fm_init(&file_manager);

    int first_index = -1;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0 ||
            strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0) {
            continue;
        }
        if (strncmp(argv[i], "--theme=", 8) == 0 || strcmp(argv[i], "--theme") == 0 || strcmp(argv[i], "-t") == 0) {
            if ((strcmp(argv[i], "--theme") == 0 || strcmp(argv[i], "-t") == 0) && i + 1 < argc)
                i++;
            continue;
        }

        load_file(&editor, NULL, argv[i]);
        if (first_index == -1)
            first_index = file_manager.active_index;
        else
            fm_switch(&file_manager, first_index);

        file_count++;
    }

    if (file_count == 0) {
        new_file(NULL);
    }

    active_file = fm_current(&file_manager);
    editor.active_file = active_file;
    editor.text_win = text_win;
    editor.file_manager = file_manager;
    editor.enable_mouse = enable_mouse;
    editor.enable_color = enable_color;

    // Show the warning dialog before entering the main editor loop
    if (app_config.show_startup_warning)
        show_warning_dialog(&editor);

    // Finishes initializing editor window and begin accepting keyboard input.
    run_editor(&editor);
    
    endwin();

    cleanup_on_exit(&file_manager);

    active_file = NULL;

    return 0;
}
