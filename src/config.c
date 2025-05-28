#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>
#include "editor.h"
#include "config.h"

int enable_color = 1; // Default to enabled

// Function to map color names to ncurses color constants
// Returns the corresponding color code for the given color name
short get_color_code(const char *color_name) {
    if (strcmp(color_name, "BLACK") == 0) return COLOR_BLACK; // Black color
    if (strcmp(color_name, "RED") == 0) return COLOR_RED; // Red color
    if (strcmp(color_name, "GREEN") == 0) return COLOR_GREEN; // Green color
    if (strcmp(color_name, "YELLOW") == 0) return COLOR_YELLOW; // Yellow color
    if (strcmp(color_name, "BLUE") == 0) return COLOR_BLUE; // Blue color
    if (strcmp(color_name, "MAGENTA") == 0) return COLOR_MAGENTA; // Magenta color
    if (strcmp(color_name, "CYAN") == 0) return COLOR_CYAN; // Cyan color
    if (strcmp(color_name, "WHITE") == 0) return COLOR_WHITE; // White color
    return -1; // Invalid color
}

void create_default_config(const char *filepath) {
    FILE *file = fopen(filepath, "w");
    if (file) {
        fprintf(file, "background_color=BLACK\n");
        fprintf(file, "keyword_color=CYAN\n");
        fprintf(file, "comment_color=GREEN\n");
        fprintf(file, "string_color=YELLOW\n");
        fprintf(file, "type_color=MAGENTA\n");
        fprintf(file, "symbol_color=RED\n");
        fprintf(file, "enable_color=true\n");
        fclose(file);
    }
}

/**
 * Reads the configuration file and applies the settings.
 * If the file doesn't exist, it creates a default configuration file.
 */
void read_config_file() {
    // Get the user's home directory
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;

    // Create the file path
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/ventorc", homedir);

    // Open the configuration file for reading
    FILE *file = fopen(filepath, "r");
    if (!file) {
        // Create default config if file doesn't exist
        create_default_config(filepath);
        file = fopen(filepath, "r");
    }

    if (!file) {
        return; // Could not create or open the file
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char *key = strtok(line, "=");
        char *value = strtok(NULL, "\n");

        if (key && value) {
            short color_code = get_color_code(value);
            if (strcmp(key, "enable_color") == 0) {
                if (strcmp(value, "true") == 0) {
                    enable_color = 1;
                    start_color();
                } else {
                    enable_color = 0;
                }
#ifdef DEBUG
                printf("Setting enable_color to %d\n", enable_color);
#endif
            } else if (enable_color) {
                if (color_code == -1) {
#ifdef DEBUG
                    printf("Invalid color value: %s\n", value);
#endif
                    continue;
                }

                if (strcmp(key, "background_color") == 0) {
                    init_pair(1, COLOR_WHITE, color_code);
#ifdef DEBUG
                    printf("Setting background_color to color code %d\n", color_code);
#endif
                } else if (strcmp(key, "keyword_color") == 0) {
                    init_pair(2, color_code, COLOR_BLACK);
#ifdef DEBUG
                    printf("Setting keyword_color to color code %d\n", color_code);
#endif
                } else if (strcmp(key, "comment_color") == 0) {
                    init_pair(3, color_code, COLOR_BLACK);
#ifdef DEBUG
                    printf("Setting comment_color to color code %d\n", color_code);
#endif
                } else if (strcmp(key, "string_color") == 0) {
                    init_pair(4, color_code, COLOR_BLACK);
#ifdef DEBUG
                    printf("Setting string_color to color code %d\n", color_code);
#endif
                } else if (strcmp(key, "type_color") == 0) {
                    init_pair(5, color_code, COLOR_BLACK);
#ifdef DEBUG
                    printf("Setting type_color to color code %d\n", color_code);
#endif
                } else if (strcmp(key, "symbol_color") == 0) {
                    init_pair(6, color_code, COLOR_BLACK);
#ifdef DEBUG
                    printf("Setting symbol_color to color code %d\n", color_code);
#endif
                }
            }
        }
    }
    fclose(file);
}
