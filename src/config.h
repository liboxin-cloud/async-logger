#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdbool.h>

typedef struct {
    char logger_level[16];
    bool output_console;
    bool output_file;
    char log_file_path[512];
    bool show_location;
}LOGGER_CONFIG_T;

typedef int CONFIG_STATUS;

CONFIG_STATUS load_logger_config(const char* filename, LOGGER_CONFIG_T* config);

CONFIG_STATUS config_level_to_enum(const char* level_str);

CONFIG_STATUS reload_logger_config(const char* filename, LOGGER_CONFIG_T* config);

#endif