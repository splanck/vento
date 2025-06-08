/*
 * Entry point for the Vento editor. Command line options are parsed
 * to determine files to load, starting line and optional color theme.
 * The editor and file manager are initialized, files are opened,
 * and finally run_editor is invoked to begin the main loop.
 */
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
#include "menu.h"
#include "ui_common.h"
#include "config.h"
#include "macro.h"
#include "editor_state.h"
#include <stdbool.h>
#include <ctype.h>

extern void load_theme(const char *name, AppConfig *cfg) __attribute__((weak));
extern void apply_colors(void) __attribute__((weak));

EditorContext editor;
extern int start_line;

/*
 * Prompt the user before exiting when unsaved changes exist.
 * Returns true to proceed with quit when no files are modified or
 * the user answers yes. Returns false otherwise.
 */
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
 * Program entry point. Processes command line options for help,
 * version, theme selection and starting line. After initializing
 * the editor and file manager an optional theme is loaded and
 * each specified file opened. If no files are given a new buffer
 * is created. The editor then calls run_editor and performs
 * cleanup when it returns.
 *
 * @param argc The number of command-line arguments.
 * @param argv An array of strings containing the command-line arguments.
 * @return 0 on successful execution.
 */
int main(int argc, char *argv[]) {
    int file_count = 0;
    const char *theme_name = NULL;
    start_line = 0;
    struct StartupMacro { char name[64]; int key; } macros[16];
    int macro_count_cli = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Usage: %s [options] [file...]\n", argv[0]);
            printf("  -h, --help     Show this help and exit\n");
            printf("  -v, --version  Print version information and exit\n");
            printf("  -t, --theme    Load a color theme before opening files\n");
            printf("  +N, --line=N   Start editing at line N\n");
            printf("      --macro=<name>=<key>  Create empty macro bound to key\n");
            return 0;
        } else if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0) {
            printf("%s\n", VERSION);
            return 0;
        } else if (strncmp(argv[i], "--theme=", 8) == 0) {
            theme_name = argv[i] + 8;
        } else if ((strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--theme") == 0) && i + 1 < argc) {
            theme_name = argv[++i];
        } else if (strncmp(argv[i], "--line=", 7) == 0) {
            start_line = atoi(argv[i] + 7);
        } else if (strncmp(argv[i], "--macro=", 8) == 0) {
            const char *spec = argv[i] + 8;
            char *eq = strchr(spec, '=');
            if (eq && macro_count_cli < 16) {
                size_t len = eq - spec;
                if (len >= sizeof(macros[0].name))
                    len = sizeof(macros[0].name) - 1;
                strncpy(macros[macro_count_cli].name, spec, len);
                macros[macro_count_cli].name[len] = '\0';
                macros[macro_count_cli].key = atoi(eq + 1);
                macro_count_cli++;
            }
        } else if (argv[i][0] == '+' && isdigit((unsigned char)argv[i][1])) {
            start_line = atoi(argv[i] + 1);
        }
    }

    initialize(&editor);

    for (int m = 0; m < macro_count_cli; ++m) {
        Macro *mac = macro_get(macros[m].name);
        if (!mac)
            mac = macro_create(macros[m].name, macros[m].key);
        else
            mac->play_key = macros[m].key;
    }

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
        if (strncmp(argv[i], "--line=", 7) == 0 || (argv[i][0] == '+' && isdigit((unsigned char)argv[i][1]))) {
            continue;
        }
        if (strncmp(argv[i], "--macro=", 8) == 0) {
            continue;
        }

        if (load_file(&editor, NULL, argv[i]) == 0) {
            if (first_index == -1) {
                first_index = file_manager.active_index;
            } else {
                FileState *loaded = fm_current(&file_manager);
                if (loaded && loaded->fp && !loaded->file_complete) {
                    loaded->file_pos = ftell(loaded->fp);
                    fclose(loaded->fp);
                    loaded->fp = NULL;
                }
                fm_switch(&file_manager, first_index);
                sync_editor_context(&editor);
            }

            file_count++;
        }
    }

    if (file_count == 0) {
        new_file(&editor, NULL);
    }

    active_file = fm_current(&file_manager);
    editor.active_file = active_file;
    editor.text_win = text_win;
    editor.file_manager = file_manager;
    editor.enable_mouse = enable_mouse;
    editor.enable_color = enable_color;

    drawBar();
    update_status_bar(&editor, editor.active_file);
    wnoutrefresh(stdscr);
    doupdate();

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
