#include "logger.h"
#include "config.h"
#include <time.h>
#include <string.h>


#define QUEUE_SIZE 65536
#define MAX_LOG_MSG 2048

#ifdef _WIN32

#else  
    #include <pthread.h>
    #include <unistd.h>
#endif

typedef struct {
    char data[MAX_LOG_MSG];
    size_t len;
    LOGGER_TYPE level;
}LOGGER_RECORD_T;

static LOGGER_RECORD_T g_queue[QUEUE_SIZE];
static atomic_size_t g_write_pos = 0;  
static atomic_size_t g_read_pos  = 0;  
static atomic_bool g_running = true; 
static LOGGER_CONFIG_T g_config; 

#ifdef _WIN32
    static HANDLE g_consumer_thrd = NULL;

#else
    #include <threads.h>
    static thrd_t g_consumer_thrd;
#endif

static const char* get_level_str(LOGGER_TYPE level);

#ifdef _WIN32
    static bool set_console_color(LOGGER_TYPE level);
    static COLOR_TYPE get_logger_color(LOGGER_TYPE level);
    static bool reset_console_color();
#endif

LOGGER_TYPE g_min_log_level = LOGGER_DEBUG;
const char* g_log_file_path = "./log_output.txt";
bool g_output_to_file = false;
bool g_output_to_console = true;
static FILE* g_fp = NULL;

#ifdef _WIN32
    static COLOR_TYPE get_logger_color(LOGGER_TYPE level) {
        switch (level) {
            case LOGGER_DEBUG: return COLOR_DEBUG; 
            case LOGGER_INFO: return COLOR_INFO; 
            case LOGGER_WARNING: return COLOR_WARNING; 
            case LOGGER_ERROR: return COLOR_ERROR; 
            case LOGGER_FATAL: return COLOR_FATAL; 
            default: {
                fprintf(stderr, "unknown logger type\n");
                return COLOR_RESET;
            }
        }
    }

    static bool set_console_color(LOGGER_TYPE level) {
        static HANDLE console_handle = NULL;
        console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
        if (console_handle == NULL) {
            fprintf(stderr, "Get output handle failed\n");
            return false;
        }

        COLOR_TYPE color = get_logger_color(level);
        SetConsoleTextAttribute(console_handle, (WORD)color);
        return true;
    }

    static bool reset_console_color() {
        static HANDLE console_handle = NULL;
        console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
        if (console_handle == NULL) {
            fprintf(stderr, "Get output handle failed\n");
            return false;
        }

        SetConsoleTextAttribute(console_handle, (WORD)COLOR_RESET);
        return true;
    }
#endif


static const char* get_level_str(LOGGER_TYPE level) {
    static const char* level_strs[] = {
        "DEBUG", "INFO", "WARNING", "ERROR", "FATAL"
    };

    if (level < LOGGER_DEBUG || level > LOGGER_FATAL) return "UNKNOWN";

    return level_strs[level];
}

static bool enqueue_log(const char* msg, size_t len, LOGGER_TYPE level);
static int consumer_thread(void* arg) {
    (void)arg;

    #define BATCH_SIZE 64
    LOGGER_RECORD_T* batch[BATCH_SIZE];
    size_t count;

    while (atomic_load(&g_running) ||
        atomic_load(&g_write_pos) != atomic_load(&g_read_pos)) {
        
        size_t read_pos = atomic_load(&g_read_pos);
        size_t write_pos = atomic_load(&g_write_pos);

        if (read_pos == write_pos) {
            #ifdef _WIN32
                Sleep(1);
            #else
                usleep(1000);
            #endif
            continue;
        }

        count = 0;
        while (count < BATCH_SIZE && 
        read_pos + count < write_pos) {
            batch[count] = &g_queue[(read_pos + count) & (QUEUE_SIZE - 1)];
            count++;
        }


        atomic_store(&g_read_pos, read_pos + count);

        for (size_t i = 0; i < count; i++) {
            LOGGER_RECORD_T* rec = batch[i];
            if (g_output_to_console && 
            rec->level >= g_min_log_level) {
                set_console_color(rec->level);
                fwrite(rec->data, 1, rec->len, stdout);
                reset_console_color();
            }
            if (g_output_to_file &&
                g_fp) {
                fwrite(rec->data, 1, rec->len, g_fp);
            }
        }
        if (g_output_to_file && g_fp && count > 0) {
            fflush(g_fp);
        }
    }

    return 0;
}

static bool enqueue_log(const char* msg, size_t len, LOGGER_TYPE level) {
    if (len == 0 || len >= MAX_LOG_MSG) {
        return false;
    }
    while (1) {
        size_t write_pos = atomic_load(&g_write_pos);
        size_t read_pos  = atomic_load(&g_read_pos);
        

        if (write_pos - read_pos >= QUEUE_SIZE) {
            return false;
        }

        size_t new_write_pos = write_pos + 1;
        if (atomic_compare_exchange_strong(&g_write_pos, &write_pos, new_write_pos)) {
            LOGGER_RECORD_T* rec = &g_queue[write_pos & (QUEUE_SIZE - 1)];
            memcpy(rec->data, msg, len);
            rec->data[len] = '\0';
            rec->len = len;
            rec->level = level;
            return true;
        }
    }
}

void logger_init(void) {

    if (load_logger_config("logger.ini", &g_config) == 0) {

        int level = config_level_to_enum(g_config.logger_level);
        atomic_store(&g_min_log_level, level);
        atomic_store(&g_output_to_console, g_config.output_console);
        atomic_store(&g_output_to_file, g_config.output_file);
        g_log_file_path = strdup(g_config.log_file_path);  
    } else {
     
        atomic_store(&g_min_log_level, LOGGER_DEBUG);
        atomic_store(&g_output_to_console, true);
        atomic_store(&g_output_to_file, false);
        g_log_file_path = "./log_output.txt";
    }

    if (atomic_load(&g_output_to_file)) {
        g_fp = fopen(g_log_file_path, "a");
        if (g_fp == NULL) {
            fprintf(stderr, "Failed to open log file: %s, disabling file output\n", g_log_file_path);
            atomic_store(&g_output_to_file, false);
        }
    }
    atomic_store(&g_running, true);


#ifdef _WIN32
    g_consumer_thrd = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)consumer_thread, NULL, 0, NULL);
    if (g_consumer_thrd == NULL) {
        fprintf(stderr, "Failed to create consumer thread\n");
        atomic_store(&g_running, false);
        if (g_fp) fclose(g_fp);
        g_fp = NULL;
        return;
    }
#else 
    if (thrd_create(&g_consumer_thrd, consumer_thread, NULL) != thrd_success) {
        fprintf(stderr, "Failed to create consumer thread\n");
        atomic_store(&g_running, false);  
        if (g_fp) {
            fclose(g_fp);
        }
        g_fp = NULL;
        return;
    }
#endif
}

void logger_shutdown(void) {
    atomic_store(&g_running, false);
#ifdef _WIN32
    if (g_consumer_thrd) {
        WaitForSingleObject(g_consumer_thrd, INFINITE);
        CloseHandle(g_consumer_thrd);
        g_consumer_thrd = NULL;
    }
#else
    if (g_consumer_thrd) {
        pthread_join(g_consumer_thrd, NULL);
        g_consumer_thrd = 0;
    }
#endif
    if (g_fp) {
        fflush(g_fp);
        fclose(g_fp);
        g_fp = NULL;
    }
}

void logger_log(LOGGER_TYPE level, const char* file, int line, const char* fmt, ...) {
    if (level < g_min_log_level) {
        return;
    }
    char time_buf[32];
    time_t now = time(NULL);
    struct tm tm_now;
#ifdef _WIN32
    localtime_s(&tm_now, &now);
#else
    localtime_r(&now, &tm_now);
#endif
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", &tm_now);

    va_list args;
    va_start(args, fmt);
    char msg_body[MAX_LOG_MSG];
    int body_len = vsnprintf(msg_body, sizeof(msg_body), fmt, args);
    va_end(args);

    if (body_len < 0) {
        body_len = 0;
    }  

    if (body_len >= MAX_LOG_MSG) {
        body_len = MAX_LOG_MSG - 1;
    }

    char line_buf[MAX_LOG_MSG + 128];
    int header_len = snprintf(line_buf, sizeof(line_buf), "[%s] [%s] [%s:%d] ",
                              time_buf, get_level_str(level), file, line);
    
    int total_len = header_len + body_len;
    int max_total_len = MAX_LOG_MSG - 1;   
    if (total_len >= max_total_len) {
        total_len = max_total_len - 1;     
        body_len = total_len - header_len;
        if (body_len < 0) body_len = 0;
    }

    memcpy(line_buf + header_len, msg_body, body_len);
    line_buf[total_len] = '\n';
    line_buf[total_len + 1] = '\0';
    
    enqueue_log(line_buf, total_len + 1, level);
}



