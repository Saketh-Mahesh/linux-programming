# Compiler to use
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -g

# Default target
all: client server

# Compile client.c to client
client: client.c
	$(CC) $(CFLAGS) -o client client.c

# Compile server.c to server
server: server.c
	$(CC) $(CFLAGS) -o server server.c

# Clean up build files
clean:
	rm -f client server
