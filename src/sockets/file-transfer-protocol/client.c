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
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <errno.h> 


#define PORT_NUM "8080" // Port number the client will connect to
#define BUFSIZE 1024    // Size of buffer for data transfer
#define BACKLOG 5           // Maximum length of pending connection queue
#define OUTPUT_FILE "output.txt"

// Function Declarations
int setup_socket(char *address);
int handle_request(int cfd);
int handle_response(int cfd, char *buf);
int handle_cleanup(int cfd, int fd);


int setup_socket(char *address) {

    int cfd, fd;                  // Client socket file descriptor
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

    if (getaddrinfo(address, PORT_NUM, &hints, &result) != 0) {
        fprintf(stderr, "Could not receive address info %s\n", strerror(errno));
        return -1;
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
        fprintf(stderr, "Could not connect socket to any address %s\n", strerror(errno));
        return -1;
    }

    freeaddrinfo(result); 

    return cfd;
}

int handle_request(int cfd) {

    char buf[BUFSIZE];
    ssize_t numRead;

     // Read from Standard Input
    memset(buf, 0, BUFSIZE);
    numRead = read(STDIN_FILENO, buf, BUFSIZE);
    if (numRead == -1) {
        fprintf(stderr, "Could not read from standard input %s\n", strerror(errno));
        return -1;
    }
    printf("Read from standard input\n");

    if (write(cfd, buf, numRead) != numRead) {
        fprintf(stderr, "Could not write to socket %s\n", strerror(errno));
        return -1;
    }

    printf("Wrote the file name to the server\n");

    return cfd;

}

int handle_response(int cfd, char *buf) {
    ssize_t numReadServer;
    int fd;
    
    fd = open(OUTPUT_FILE, O_RDWR | O_CREAT | O_TRUNC,
              S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
              S_IROTH | S_IWOTH);
    
    if (fd == -1) {
        fprintf(stderr, "Could not open output file %s\n", strerror(errno));
        return -1;
    }

    while ((numReadServer = read(cfd, buf, BUFSIZE)) > 0) {
        printf("Received %zd bytes from server\n", numReadServer);
        if (write(fd, buf, numReadServer) == -1) {
            fprintf(stderr, "Could not write file contents back to client socket %s\n", strerror(errno));
            return -1;
        }
        printf("Wrote %zd bytes to server\n", numReadServer);
    }
    printf("Finished reading from server with status: %zd\n", numReadServer);
    
    return fd;
}

int handle_cleanup(int cfd, int fd) {
    if (close(cfd) == -1) {
        fprintf(stderr, "Could not close socket %s\n", strerror(errno));
        return -1;
    }
    printf("Closed the client socket\n");

    if (close(fd) == -1) {
        fprintf(stderr, "Could not close file %s\n", strerror(errno));
        return -1;
    }
    printf("Closed the file descriptor\n");
    
    return 0;
}

int main(int argc, char *argv[])
{
    int cfd, fd;  
    char buf[BUFSIZE];


    cfd = setup_socket(argv[1]);
    if (cfd == -1)
        return -1;
    
    // Handle request
    if (handle_request(cfd) == -1)
        return -1;

    // Handle response
    fd = handle_response(cfd, buf);
    if (fd == -1)
        return -1;

    // Cleanup
    return handle_cleanup(cfd, fd);
}