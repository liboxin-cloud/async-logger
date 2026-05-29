#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static void trim(char* str);
static CONFIG_STATUS parse_line(char* line, char* key, char* value);
static bool str_to_bool(const char* str);
static CONFIG_STATUS create_default_config(const char* filename);

static void trim(char* str) {
    if (str == NULL) {
        fprintf(stderr, "the str is NULL\n");
        return;
    }
    char* start = str;
    char* end = NULL;

    while ((unsigned char)*start == ' ') {
        start++;
    }   

    if (*start == 0) {
        str[0] = '\0';
        return;
    }

    end = start + strlen(start) - 1;

    while (end > start &&
    (unsigned char)*end == ' ') {
        end--;
    }

    end[1] = '\0';

    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

static CONFIG_STATUS parse_line(char* line, char* key, char* value) {
    if (line == NULL ||
    key == NULL ||
    value == NULL) {
        fprintf(stderr, "args is wrong\n");
        return -1;
    }

    char* eq = strchr(line, '=');
    if (eq == NULL) {
        return -1;
    }

    *eq = '\0';
    strcpy(key, line);
    strcpy(value, eq + 1);

    trim(key);
    trim(value);


    if (value[0] == '"' || value[0] == '\'') {
        size_t len = strlen(value);
        if (len > 1 && (value[len-1] == '"' || value[len-1] == '\'')) {
            value[len-1] = '\0';
            memmove(value, value + 1, len - 1);
        }
    }

    return 0;
}

static bool str_to_bool(const char* str) {
    char lower[16] = {0};
    size_t i;
    for (i = 0; str[i] && i < sizeof(lower)-1; i++) {
        lower[i] = tolower(str[i]);
    }

    return (strcmp(lower, "true") == 0 ||
            strcmp(lower, "yes") == 0 ||
            strcmp(lower, "1") == 0);

}


CONFIG_STATUS load_logger_config(const char* filename, LOGGER_CONFIG_T* config) {
    if (filename == NULL) {
        fprintf(stderr, "filename is NULL\n");
        return -1;
    }

    if (config == NULL) {
        fprintf(stderr, "config is NULL\n");
        return -1;
    }

    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Config file '%s' not found, creating default...\n", filename);
        if (create_default_config(filename) != 0) {
            fprintf(stderr, "Failed to create default config file, using built-in defaults\n");
        }

        fp = fopen(filename, "r");
        if (fp == NULL) {
            fprintf(stderr, "Still cannot open config file, using built-in defaults\n");
            strcpy(config->logger_level, "DEBUG");
            config->output_console = true;
            config->output_file = false;
            strcpy(config->log_file_path, "./log_output.txt");
            return -1; 
        }
    }

    strcpy(config->logger_level, "DEBUG");
    config->output_console = true;
    config->output_file = false;
    strcpy(config->log_file_path, "./log_output.txt");
    
    char line[512];
    char key[128], value[256];
    char current_section[64] = "";

    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\r\n")] = '\0';


        if (line[0] == '\0' || line[0] == ';' || line[0] == '#') {
            continue;
        }

        if (line[0] == '[') {
            char* end = strchr(line, ']');
            if (end) {
                *end = '\0';
                strcpy(current_section, line + 1);
            }
            continue;
        }

        if (strcmp(current_section, "logger") != 0) {
            continue;
        }
        
        if (parse_line(line, key, value) == 0) {
            if (strcmp(key, "log_level") == 0) {
                strcpy(config->logger_level, value);
            } else if (strcmp(key, "output_console") == 0) {
                config->output_console = str_to_bool(value);
            } else if (strcmp(key, "output_file") == 0) {
                config->output_file = str_to_bool(value);
            } else if (strcmp(key, "log_file_path") == 0) {
                strcpy(config->log_file_path, value);
            } else {
                fprintf(stderr, "Warning: Unknown config key '%s' in logger.ini\n", key);
            }
        }
    }

    fclose(fp);
    return 0;
}

CONFIG_STATUS config_level_to_enum(const char* level_str) {
    if (level_str == NULL) {
        fprintf(stderr, "level_str is NULL\n");
        return -1;
    }

    if (strcmp(level_str, "DEBUG") == 0) {
        return 0;
    }

    if (strcmp(level_str, "INFO") == 0) {
        return 1;
    }    

    if (strcmp(level_str, "WARNING") == 0) {
        return 2;
    }
        
    if (strcmp(level_str, "ERROR") == 0) {
        return 3;
    }
        
    if (strcmp(level_str, "FATAL") == 0) {
        return 4;
    }

    return -1; 
}

static CONFIG_STATUS create_default_config(const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        fprintf(stderr, "Failed to create default config file: %s\n", filename);
        return -1;
    }
    
    fprintf(fp, "; Logger Configuration File\n");
    fprintf(fp, "; Generated automatically by logger system\n\n");
    
    fprintf(fp, "[logger]\n");
    fprintf(fp, "; logger level: DEBUG, INFO, WARNING, ERROR, FATAL\n");
    fprintf(fp, "log_level = DEBUG\n\n");
    
    fprintf(fp, "; output to console: true, false, yes, no, 1, 0\n");
    fprintf(fp, "output_console = true\n\n");
    
    fprintf(fp, "; output to file: true, false, yes, no, 1, 0\n");
    fprintf(fp, "output_file = false\n\n");
    
    fprintf(fp, "; logger file path(relative path)\n");
    fprintf(fp, "log_file_path = ./log_output.txt\n\n");
    
    fprintf(fp, "; [INFO]: modify the ini file and restart the program to work\n");
    
    fclose(fp);
    printf("Created default configuration file: %s\n", filename);
    return 0;
}

CONFIG_STATUS reload_logger_config(const char* filename, LOGGER_CONFIG_T* config) {
    return load_logger_config(filename, config);
}
