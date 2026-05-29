
SRC_DIR = ../../
BIN_DIR = ../../bin
OBJ_DIR = $(BIN_DIR)/obj


CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -I$(SRC_DIR)/src -D_POSIX_C_SOURCE=200809L
LDFLAGS = -lpthread


TARGET = $(BIN_DIR)/test_logger
OBJS = $(OBJ_DIR)/logger.o $(OBJ_DIR)/config.o $(OBJ_DIR)/test_logger.o


all: $(OBJ_DIR) $(TARGET)


$(OBJ_DIR):
	mkdir -p $(BIN_DIR)
	mkdir -p $(OBJ_DIR)


$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Build complete: $(TARGET)"


$(OBJ_DIR)/logger.o: $(SRC_DIR)/src/logger.c $(SRC_DIR)/src/logger.h
	$(CC) $(CFLAGS) -c $< -o $@


$(OBJ_DIR)/config.o: $(SRC_DIR)/src/config.c $(SRC_DIR)/src/config.h
	$(CC) $(CFLAGS) -c $< -o $@


$(OBJ_DIR)/test_logger.o: $(SRC_DIR)/src/test_logger.c $(SRC_DIR)/src/logger.h
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -rf $(BIN_DIR)
	@echo "Cleaned: $(BIN_DIR)"


run: $(TARGET)
	@echo "Running $(TARGET)..."
	@echo ""
	$(TARGET)


debug: CFLAGS += -g -O0 -DDEBUG
debug: clean all

.PHONY: all clean run debug