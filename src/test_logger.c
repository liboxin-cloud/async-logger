#include "logger.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
    #include <windows.h>
    #define THREAD_FUNC DWORD WINAPI
    typedef HANDLE thread_t;
#else
    #include <pthread.h>
    #define THREAD_FUNC void*
    typedef pthread_t thread_t;
#endif

#define THREAD_COUNT 8
#define LOGS_PER_THREAD 10000

#ifdef _WIN32
    DWORD WINAPI thread_func(LPVOID arg) {
        int id = *(int*)arg;
#else
    void* thread_func(void* arg) {
        int id = *(int*)arg;
#endif
        for (int i = 0; i < LOGS_PER_THREAD; ++i) {
            LOG_DEBUG("Thread %d debug message #%d", id, i);
            LOG_INFO("Thread %d info message #%d", id, i);
            LOG_WARNING("Thread %d warning message #%d", id, i);
            LOG_ERROR("Thread %d error message #%d", id, i);
            if (i % 5000 == 0) {
                LOG_FATAL("Thread %d fatal message #%d", id, i);
            }
        }
        return 0;
    }

int main(void) {

    logger_init();

    thread_t threads[THREAD_COUNT];
    int ids[THREAD_COUNT];

    printf("Starting %d threads, each writing %d log messages...\n",
           THREAD_COUNT, LOGS_PER_THREAD * 5);

    for (int i = 0; i < THREAD_COUNT; ++i) {
        ids[i] = i;
#ifdef _WIN32
        threads[i] = CreateThread(NULL, 0, thread_func, &ids[i], 0, NULL);
        if (threads[i] == NULL) {
            fprintf(stderr, "Failed to create thread %d\n", i);
            return 1;
        }
#else
        if (pthread_create(&threads[i], NULL, thread_func, &ids[i]) != 0) {
            fprintf(stderr, "Failed to create thread %d\n", i);
            return 1;
        }
#endif
    }

    for (int i = 0; i < THREAD_COUNT; ++i) {
#ifdef _WIN32
        WaitForSingleObject(threads[i], INFINITE);
        CloseHandle(threads[i]);
#else
        pthread_join(threads[i], NULL);
#endif
    }

    logger_shutdown();
    printf("All logs generated. Check './log_output.txt' and console output.\n");
    return 0;
}