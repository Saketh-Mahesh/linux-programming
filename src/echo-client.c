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
    ssize_t numRead, numReadServer;              // Number of bytes read
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

    if (getaddrinfo(argv[1], PORT_NUM, &hints, &result) != 0) {
        fprintf(stderr, "Could not receive address info\n");
        exit(1);
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {

        cfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (cfd == -1)
            continue;                   /* On error, try next address */

        if (connect(cfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;                              /* Success */
        /* Connect failed: close this socket and try next address */
        close(cfd);
    }

    if (rp == NULL) {
        fprintf(stderr, "Could not connect socket to any address\n");
        exit(1);
    }

    freeaddrinfo(result); 

    numRead = read(STDIN_FILENO, buf, BUFSIZE);
    if (numRead == -1) {
        fprintf(stderr, "Could not read from standard input\n");
        exit(1);
    }

    if (write(cfd, buf, numRead) != numRead) {
        fprintf(stderr, "Could not write to socket\n");
        exit(1);
    }

    numReadServer = read(cfd, buf, BUFSIZE);
    if (numReadServer == -1) {
        fprintf(stderr, "Could not read echo statement back from server\n");
        exit(1);
    }

    /* Write the response to standard output */
    if (write(STDOUT_FILENO, buf, numReadServer) != numReadServer) {
        fprintf(stderr, "partial/failed write to stdout\n");
        exit(1);
    }  


    if (close(cfd) == -1) {
        fprintf(stderr, "Could not close socket\n");
        exit(1);
    } 
}
