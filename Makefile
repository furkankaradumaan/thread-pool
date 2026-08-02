CC = gcc

CFLAGS = -Wall -Wextra -g \
          -Wsign-conversion -Wconversion

LDLIBS = -lcrypto -pthread

SANITIZERS = -fsanitize=address,undefined

INCLUDES = -Iinclude \
           -Iexternal/job-queue/include

SRC_DIR = src
QUEUE_SRC_DIR = external/job-queue/src

OBJ_DIR = obj

SRCS = $(wildcard $(SRC_DIR)/*.c) \
       $(wildcard $(QUEUE_SRC_DIR)/*.c)

OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRCS))

TARGET = thread-pool-demo

all: $(OBJ_DIR) $(TARGET)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)
	mkdir -p $(OBJ_DIR)/$(SRC_DIR)
	mkdir -p $(OBJ_DIR)/$(QUEUE_SRC_DIR)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(SANITIZERS) demo/main.c $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean
