# Async Logger - High Performance Cross-Platform C Logging Library

A high-performance, cross-platform, asynchronous logging library written in C11. Features lock-free queue, multi-thread support, configurable outputs, and colored console output.

## Features

- High Performance: Lock-free ring buffer with CAS operations, non-blocking for producer threads
- Asynchronous: Background consumer thread handles all I/O operations
- Multi-Thread Safe: Supports concurrent logging from multiple threads without locks
- Colored Output: ANSI colors on Linux/macOS, native colors on Windows console
- Configurable: INI file configuration with auto-generated defaults
- Log Levels: DEBUG, INFO, WARNING, ERROR, FATAL with runtime filtering
- Dual Output: Simultaneous output to console (colored) and file (plain text)
- Batch Processing: Processes logs in batches for reduced system calls
- Cross-Platform: Works on Windows (MinGW), Linux, and macOS

## Requirements

- C11 compatible compiler (GCC, Clang, MinGW-w64)
- CMake 3.10+ (optional, for CMake build)
- POSIX threads (Linux/macOS) or winpthreads (Windows)



## Quick Start

### Directory Structure
project/
├── src/ # Source files
│ ├── logger.h
│ ├── logger.c
│ ├── config.h
│ ├── config.c
│ └── test_logger.c # Example usage
├── bin/ # Build output (auto-created)
│ ├── obj/ # Object files
│ └── test_logger # Executable
└── build/
├── Windows/ # Windows Makefile
└── Linux/ # Linux Makefile


### Building

#### Windows (MinGW-w64)
```bash
cd build/Windows
mingw32-make
mingw32-make run
```

#### Linux / macOS
``` bash
cd build/Linux
make
make run

Manual Build
```
### Create directories
mkdir -p bin/obj

### Compile
gcc -std=c11 -Wall -Wextra -O2 -Isrc -D_POSIX_C_SOURCE=200809L -c src/logger.c -o bin/obj/logger.o
gcc -std=c11 -Wall -Wextra -O2 -Isrc -D_POSIX_C_SOURCE=200809L -c src/config.c -o bin/obj/config.o
gcc -std=c11 -Wall -Wextra -O2 -Isrc -D_POSIX_C_SOURCE=200809L -c src/test_logger.c -o bin/obj/test_logger.o
gcc -o bin/test_logger bin/obj/logger.o bin/obj/config.o bin/obj/test_logger.o -lpthread

### Run
```
./bin/test_logger
```
### Usage

#### Basic Example
```
#include "logger.h"
int main(void) {
    // Configuration (optional, can also use logger.ini)
    atomic_store(&g_min_log_level, LOGGER_DEBUG);
    atomic_store(&g_output_to_console, true);
    atomic_store(&g_output_to_file, true);
    
    logger_init();
    
    LOG_DEBUG("Debug message: value = %d", 42);
    LOG_INFO("System initialized successfully");
    LOG_WARNING("Disk usage: %d%%", 85);
    LOG_ERROR("Failed to open file: %s", "no such file");
    LOG_FATAL("Unrecoverable error, exiting");
    
    logger_shutdown();
    return 0;
}
```
#### Multi-Thread Example
```
#include "logger.h"
#include <threads.h>

int thread_func(void* arg) {
    int id = *(int*)arg;
    for (int i = 0; i < 1000; i++) {
        LOG_INFO("Thread %d: iteration %d", id, i);
    }
    return 0;
}

int main(void) {
    logger_init();
    
    thrd_t t1, t2;
    int id1 = 1, id2 = 2;
    thrd_create(&t1, thread_func, &id1);
    thrd_create(&t2, thread_func, &id2);
    
    thrd_join(t1, NULL);
    thrd_join(t2, NULL);
    
    logger_shutdown();
    return 0;
}
```

### Configuration

Create logger.ini in the working directory:

ini:
```
; Logger Configuration File
[logger]
; Log level: DEBUG, INFO, WARNING, ERROR, FATAL
log_level = INFO

; Output to console: true, false, yes, no, 1, 0
output_console = true

; Output to file: true, false, yes, no, 1, 0
output_file = true

; Log file path (relative or absolute)
log_file_path = ./log_output.txt

```
If no configuration file exists, a default one is automatically created.

### Runtime Configuration

```
// Change log level at runtime
atomic_store(&g_min_log_level, LOGGER_WARNING);

// Enable/disable outputs dynamically
atomic_store(&g_output_to_console, false);
atomic_store(&g_output_to_file, true);
```

### Log Output Format

```
[2026-01-15 10:30:45] [INFO] [main.c:25] Application started
[2026-01-15 10:30:46] [WARNING] [main.c:30] Memory usage high: 85%
[2026-01-15 10:30:47] [ERROR] [main.c:35] Connection failed: timeout
```

### Architecture
```
+-------------+     +------------------+     +-----------------+
|  Thread 1   |     |                  |     |   Console       |
|  LOG_XXX()  |---->|                  |---->|   (Colored)     |
+-------------+     |  Lock-Free Queue |     +-----------------+
+-------------+     |  (Ring Buffer)   |     +-----------------+
|  Thread 2   |---->|                  |---->|     File        |
|  LOG_XXX()  |     |                  |     |   (Plain text)  |
+-------------+     +------------------+     +-----------------+
+-------------+              |
|  Thread N   |              v
|  LOG_XXX()  |---->   Background Consumer Thread
+-------------+         (Batch Processing)
```
### Key Components
```
Lock-Free Queue: MPSC (Multiple Producer, Single Consumer) ring buffer using CAS operations

Async Consumer: Dedicated thread for I/O operations, never blocks producers

Batch Processing: Processes up to 64 logs per batch to reduce system calls

Config Module: INI file parsing with automatic default generation

```
### API Reference
#### Functions
```
Function	Description
logger_init()	Initialize the logging system (start consumer thread)
logger_shutdown()	Shutdown logging system (flush and cleanup)
logger_log(level, file, line, fmt, ...)	Internal logging function (use macros instead)
```

### Macros
```
Macro	Description
LOG_DEBUG(fmt, ...)	    Log DEBUG level message
LOG_INFO(fmt, ...)	    Log INFO level message
LOG_WARNING(fmt, ...)	Log WARNING level message
LOG_ERROR(fmt, ...)	    Log ERROR level message
LOG_FATAL(fmt, ...)	    Log FATAL level message
```
### Global Configuration Variables
```
    Variable	       Type	                Description
g_min_log_level	    atomic_int	    Minimum log level to output
g_output_to_console	atomic_bool	    Enable/disable console output
g_output_to_file	atomic_bool	    Enable/disable file output
g_log_file_path	    const char*	    Path to log file
```

### License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.


### Acknowledgments

    Lock-free queue implementation based on classic ring buffer design

    Cross-platform thread abstraction using condition compilation