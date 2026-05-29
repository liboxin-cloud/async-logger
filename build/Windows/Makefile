
SRC_DIR = ..\..\src
BIN_DIR = ..\..\bin
OBJ_DIR = $(BIN_DIR)\obj


CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -I$(SRC_DIR) -D_WIN32_WINNT=0x0600
LDFLAGS = -lpthread


TARGET = $(BIN_DIR)\test_logger.exe
OBJS = $(OBJ_DIR)\logger.o $(OBJ_DIR)\config.o $(OBJ_DIR)\test_logger.o


all: $(OBJ_DIR) $(TARGET)


$(OBJ_DIR):
	if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)


$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo Build complete: $(TARGET)


$(OBJ_DIR)\logger.o: $(SRC_DIR)\logger.c $(SRC_DIR)\logger.h
	$(CC) $(CFLAGS) -c $< -o $@


$(OBJ_DIR)\config.o: $(SRC_DIR)\config.c $(SRC_DIR)\config.h
	$(CC) $(CFLAGS) -c $< -o $@


$(OBJ_DIR)\test_logger.o: $(SRC_DIR)\test_logger.c $(SRC_DIR)\logger.h
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	if exist $(BIN_DIR) rmdir /s /q $(BIN_DIR)
	@echo Cleaned: $(BIN_DIR)

run: $(TARGET)
	@echo Running $(TARGET)...
	@echo.
	$(TARGET)


debug: CFLAGS += -g -O0 -DDEBUG
debug: clean all

.PHONY: all clean run debug