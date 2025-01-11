#define _DEFAULT_SOURCE
// Required header files for socket programming
#include <sys/types.h>  // Basic system data types
#include <sys/socket.h> // Core socket functions and data structures
#include <netinet/in.h> // Internet address family and structures
#include <arpa/inet.h>  // inet_addr and related functions
#include <netdb.h>      // Network database operations
#include <unistd.h>     // POSIX operating system API (for close())
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>


#define PORT_NUM "8080" // Port number the client will connect to
#define BUFSIZE 1024    // Size of buffer for data transfer
#define BACKLOG 5           // Maximum length of pending connection queue

int main(int argc, char *argv[])
{
    int lfd, cfd;                      // Client socket file descriptor
    char buf[BUFSIZE];            // Buffer for data transfer
    ssize_t numRead;              // Number of bytes read
    socklen_t addrlen;              // Length of client address structure
    struct addrinfo hints;        // Used to specify socket criteria
    struct addrinfo *result, *rp; // Will hold the address info results
    struct sockaddr_storage claddr; // Client address structure (IPv4 or IPv6)

    // Initialize the hints structure to zero
    memset(&hints, 0, sizeof(struct addrinfo));
    // The following three assignments are redundant after memset
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    hints.ai_family = AF_UNSPEC;     // Allow both IPv4 and IPv6
    hints.ai_socktype = SOCK_STREAM; // Specify TCP stream sockets
    hints.ai_flags = AI_NUMERICSERV; // Port number is numeric

    // Get address information for the server (argv[1] should be server address)
    if (getaddrinfo(argv[1], PORT_NUM, &hints, &result) != 0)
        printf("getaddrinfo failed\n");

    // Try each returned address until we successfully connect
    // Try each returned address until we successfully bind
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        // Create a socket with the current address family
        lfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (lfd == -1)
            continue;               // Try next address if socket creation failed

        int reuseaddr = 1;
        if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr)) == -1)
            fprintf(stderr, "Could not set SO_REUSEADDR option\n");
        

        // Try to bind the socket to the address
        if (bind(lfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;                  // Success - break out of loop

        // bind() failed - try next address
    }

    // Check if we failed to bind to any address
    if (rp == NULL) {
        fprintf(stderr, "Could not bind socket to any address\n");
        exit(1);
    }

    // Mark socket as accepting connections
    if (listen(lfd, BACKLOG) == -1) {
        fprintf(stderr, "Could not listen properly\n");
        exit(1);
    }

    // Free the address information structure
    freeaddrinfo(result);

    while (1) {

        addrlen = sizeof(struct sockaddr_storage);
        // Accept a client connection
        cfd = accept(lfd, (struct sockaddr *) &claddr, &addrlen);
        if (cfd == -1) {
            fprintf(stderr, "Could not accept connection\n");
            exit(1);
        }

        numRead = read(cfd, buf, BUFSIZE - 1);
        buf[numRead] = '\0';
        if (numRead > 0) {
            printf("Received: %s\n", buf);
        }
        else {
            fprintf(stderr, "Could not read data from client\n");
            exit(1);
        }

        for (size_t i = 0; i <= strlen(buf); i++) {
            buf[i] = toupper(buf[i]);
        }

        if (write(cfd, buf, BUFSIZE) == -1) {
            fprintf(stderr, "Could not echo back to client\n");
            exit(1);
        }

        if (close(cfd) == -1) {
            fprintf(stderr, "Could not close client socket\n");
            exit(1);
        }
    }

    if (close(lfd) == -1) {
        fprintf(stderr, "Could not close listening socket\n");
        exit(1);
    }
}
