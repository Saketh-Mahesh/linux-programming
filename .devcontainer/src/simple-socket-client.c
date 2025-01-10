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

#define PORT_NUM "8080" // Port number the client will connect to
#define BUFSIZE 1024    // Size of buffer for data transfer

int main(int argc, char *argv[])
{
    int cfd;                      // Client socket file descriptor
    char buf[BUFSIZE];            // Buffer for data transfer
    ssize_t numRead;              // Number of bytes read
    struct addrinfo hints;        // Used to specify socket criteria
    struct addrinfo *result, *rp; // Will hold the address info results

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
    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        // Create a socket with the current address family
        cfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (cfd == -1)
            continue; // Try next address if socket creation failed

        // Attempt to connect to the server
        if (connect(cfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break; // Success - break out of loop

        // Connection failed - close socket and try next address
        close(cfd);
    }

    // Check if we failed to connect to any address
    if (rp == NULL)
        printf("Could not connect socket to any address\n");

    // Free the address information structure
    freeaddrinfo(result);

    printf("Connected to the server. I am running on Codespace!!!\n");
}
