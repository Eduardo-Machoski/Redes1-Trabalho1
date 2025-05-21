# Compiler and flags
CC = gcc
CFLAGS = -Wextra -DDEBUG

# Directories
SRC_DIR = src
TMP_DIR = tmp
BIN_DIR = .

# Source files
CLIENT_SRCS = $(SRC_DIR)/main_client.c $(SRC_DIR)/client_control.c $(SRC_DIR)/connection_protocol.c
SERVER_SRCS = $(SRC_DIR)/main_server.c $(SRC_DIR)/server_control.c $(SRC_DIR)/connection_protocol.c

# Object files (in tmp/)
CLIENT_OBJS = $(patsubst $(SRC_DIR)/%.c,$(TMP_DIR)/%.o,$(CLIENT_SRCS))
SERVER_OBJS = $(patsubst $(SRC_DIR)/%.c,$(TMP_DIR)/%.o,$(SERVER_SRCS))

# purges and then remakes everything
remake: purge all

# Targets
all: client server

client: $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/client $(CLIENT_OBJS)

server: $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/server $(SERVER_OBJS)

# Create tmp folder if not exists and compile
$(TMP_DIR)/%.o: $(SRC_DIR)/%.c | $(TMP_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure tmp folder exists
$(TMP_DIR):
	mkdir -p $(TMP_DIR)

# Cleaning rules
clean: clean-client clean-server

clean-client:
	rm -f $(CLIENT_OBJS)

clean-server:
	rm -f $(SERVER_OBJS)

purge: purge-client purge-server
	rmdir $(BIN_DIR)/tmp

purge-client: clean-client
	rm -f $(BIN_DIR)/client

purge-server: clean-server
	rm -f $(BIN_DIR)/server

.PHONY: all clean clean-client clean-server purge purge-client purge-server

