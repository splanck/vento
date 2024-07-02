#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>
#include "editor.h"

// Function to map color names to ncurses color constants
short get_color_code(const char *color_name) {
    if (strcmp(color_name, "BLACK") == 0) return COLOR_BLACK;
    if (strcmp(color_name, "RED") == 0) return COLOR_RED;
    if (strcmp(color_name, "GREEN") == 0) return COLOR_GREEN;
    if (strcmp(color_name, "YELLOW") == 0) return COLOR_YELLOW;
    if (strcmp(color_name, "BLUE") == 0) return COLOR_BLUE;
    if (strcmp(color_name, "MAGENTA") == 0) return COLOR_MAGENTA;
    if (strcmp(color_name, "CYAN") == 0) return COLOR_CYAN;
    if (strcmp(color_name, "WHITE") == 0) return COLOR_WHITE;
    return COLOR_WHITE; // Default color
}

void read_config_file() {
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/vento.ctl", homedir);

    FILE *file = fopen(filepath, "r");
    if (!file) {
        return; // File doesn't exist, use default settings
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char *key = strtok(line, "=");
        char *value = strtok(NULL, "\n");

        if (key && value) {
            if (strcmp(key, "background_color") == 0) {
                init_pair(1, get_color_code(value), get_color_code(value));
            } else if (strcmp(key, "keyword_color") == 0) {
                init_pair(2, get_color_code(value), COLOR_BLACK);
            } else if (strcmp(key, "comment_color") == 0) {
                init_pair(3, get_color_code(value), COLOR_BLACK);
            } else if (strcmp(key, "string_color") == 0) {
                init_pair(4, get_color_code(value), COLOR_BLACK);
            } else if (strcmp(key, "type_color") == 0) {
                init_pair(5, get_color_code(value), COLOR_BLACK);
            } else if (strcmp(key, "symbol_color") == 0) {
                init_pair(6, get_color_code(value), COLOR_BLACK);
            }
        }
    }

    fclose(file);
}
