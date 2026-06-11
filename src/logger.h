#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <stdbool.h>

#ifdef __cplusplus
    extern "C" {
#endif

#ifdef __cplusplus
    #include <atomic>
    typedef std::atomic_bool atomic_bool;
    typedef std::atomic_int atomic_int;
#else
    #include <stdatomic.h>
#endif

typedef enum {
    LOGGER_DEBUG,
    LOGGER_INFO, 
    LOGGER_WARNING,
    LOGGER_ERROR,
    LOGGER_FATAL
}LOGGER_TYPE;

#ifdef _WIN32
    #include <windows.h>


    typedef enum {
        COLOR_RESET = 7, 
        COLOR_DEBUG = 11,
        COLOR_INFO = 10,
        COLOR_WARNING = 14,
        COLOR_ERROR = 12,
        COLOR_FATAL = 13 
    }COLOR_TYPE;

#elif defined(__linux__) || defined(__unix__)

    #define COLOR_RESET   "\033[0m"
    #define COLOR_DEBUG   "\033[36m"   
    #define COLOR_INFO    "\033[32m"   
    #define COLOR_WARNING "\033[33m"   
    #define COLOR_ERROR   "\033[31m"   
    #define COLOR_FATAL   "\033[35m"   

    #define set_console_color(level) \
        do {\
            switch (level) {\
                case LOGGER_DEBUG: {\
                    printf(COLOR_DEBUG);\
                    break;\
                }\
                case LOGGER_INFO: {\
                    printf(COLOR_INFO);\
                    break;\
                }\
                case LOGGER_WARNING: {\
                    printf(COLOR_WARNING);\
                    break;\
                }\
                case LOGGER_ERROR: {\
                    printf(COLOR_ERROR);\
                    break;\
                }\
                case LOGGER_FATAL: {\
                    printf(COLOR_FATAL);\
                    break;\
                }\
                default: {\
                    break;\
                }\
            }\
        } while (0)

    #define reset_console_color() printf(COLOR_RESET)
#else
    #error "Unsupported platform"
#endif


extern LOGGER_TYPE g_min_log_level;      
extern bool g_output_to_console;         
extern bool g_output_to_file;            
extern const char* g_log_file_path;      
extern atomic_bool g_show_location;

void logger_init(void);
void logger_shutdown(void);
void logger_log(LOGGER_TYPE level, const char* file, int line, const char* fmt, ...);

#define LOG_DEBUG(...)      logger_log(LOGGER_DEBUG, __FILE__, __LINE__, __VA_ARGS__) 
#define LOG_INFO(...)       logger_log(LOGGER_INFO, __FILE__, __LINE__, __VA_ARGS__) 
#define LOG_WARNING(...)    logger_log(LOGGER_WARNING, __FILE__, __LINE__, __VA_ARGS__)         
#define LOG_ERROR(...)      logger_log(LOGGER_ERROR, __FILE__, __LINE__, __VA_ARGS__) 
#define LOG_FATAL(...)      logger_log(LOGGER_FATAL, __FILE__, __LINE__, __VA_ARGS__) 

#ifdef __cplusplus
    }
#endif

#endif