# Compiler to use
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -g

# Default target
all: echo-client echo-server

# Compile client.c to client
client: echo-client.c
	$(CC) $(CFLAGS) -o echo-client echo-client.c

# Compile server.c to server
server: echo-server.c
	$(CC) $(CFLAGS) -o echo-server echo-server.c

# Clean up build files
clean:
	rm -f echo-client echo-server
