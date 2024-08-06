#ifndef CONFIG_H
#define CONFIG_H

#define VERSION "0.1.1"

extern int enable_color;

short get_color_code(const char *color_name);
void read_config_file();

#endif // CONFIG_H
