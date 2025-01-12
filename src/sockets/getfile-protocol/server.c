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
#define BUFSIZE 2048    // Size of buffer for data transfer
#define BACKLOG 5           // Maximum length of pending connection queue

int setup_socket(char *address) {

    int lfd;         
    struct addrinfo hints;
    struct addrinfo *result, *rp; 
    struct sockaddr_storage claddr;

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
    if (getaddrinfo(address, PORT_NUM, &hints, &result) != 0) {
        fprintf(stderr, "Could not get addresss info %s\n", strerror(errno));
        return -1;
    }

    // Try each returned address until we successfully connect
    // Try each returned address until we successfully bind
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        // Create a socket with the current address family
        lfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (lfd == -1)
            continue;               // Try next address if socket creation failed

        int reuseaddr = 1;
        if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr)) == -1) {
            fprintf(stderr, "Could not set SO_REUSEADDR option %s\n", strerror(errno));
            return -1;
        }
        
        // Try to bind the socket to the address
        if (bind(lfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;                  // Success - break out of loop

        // bind() failed - try next address
    }
    // Check if we failed to bind to any address
    if (rp == NULL) {
        fprintf(stderr, "Could not bind socket to any address %s\n", strerror(errno));
        return -1;
    }

    // Mark socket as accepting connections
    if (listen(lfd, BACKLOG) == -1) {
        fprintf(stderr, "Could not listen properly %s\n", strerror(errno));
        exit(1);
    }

    // Free the address information structure
    freeaddrinfo(result);

    return lfd;
}

int handle_parsing_protocol(char *requestBuf, char *clientBytesRead, char *fileName) {

    char *method, *scheme, *fileName, *endScheme;   

    // process the request buffer
    int startIndex = 0; 
    int spaceCounter = 0;         
    for (ssize_t i = 0; i < clientBytesRead; i++) {
        if (requestBuf[i] == ' ') {
            requestBuf[i] = '\0';
            if (spaceCounter == 0) {
                scheme = &requestBuf[startIndex];
                startIndex = i+1;
            }   
            else if (spaceCounter == 1) {
                method = &requestBuf[startIndex];
                startIndex = i+1;
            }
            spaceCounter++;
        }
    }

    // check if method or scheme were never populated
    if (method == NULL || scheme == NULL) {
        printf("The method or scheme is missing\n");
        return -1;
    }

    // find the location of the ending scheme
    endScheme = strstr(requestBuf, "\r\n\r\n");
    if (endScheme == NULL) {
        printf("The ending scheme of the request is missing\n");
        return -1;
    }

    // null terminate 
    *endScheme = '\0';
    // set the filename to be the starting point of the index (which should have been set to the point right after the method)
    fileName = &requestBuf[startIndex];

    // Error Checking
    if (strcmp(scheme, "GETFILE") != 0) {
        printf("The given scheme does not match GETFILE\n");
        return -1;
    }
    if (strcmp(method, "GET") != 0) {
        printf("The given method does not match GETFILE\n");
        return -1;
    }

    if (fileName == NULL) {
        printf("The filename is missing\n");
        return -1;
    }

    if (spaceCounter != 2) {
        printf("Input is not formatted properly. Ensure that the scheme, method, and filepath are separated by a space\n");
        return -1;
    }

    return 0;
}


int handle_file_transfer(int cfd) {

    int fd, fileSize;
    char buf[BUFSIZE], requestBuf[BUFSIZE];      
    ssize_t clientBytesRead, fileBytesRead;  
    char *method, *scheme, *fileName, *endScheme, *header, *status;

    clientBytesRead = read(cfd, requestBuf, BUFSIZE);
    if (clientBytesRead > 0) {
        printf("Bytes read from client: %zd\n", clientBytesRead);
        printf("Raw received data: ");
        for (int i = 0; i < clientBytesRead; i++) {
            printf("%c[%d] ", requestBuf[i], requestBuf[i]);
        }
        printf("\n");
    }

    if (handle_parsing_protocol(requestBuf, clientBytesRead, fileName) != 0) {
        printf("The parsing protocol failed\n");
        return -1;
    }

    if (access(fileName, R_OK) != 0) {
        fprintf(stderr, "Error while checking the file", strerror(errno));
        status = "FILE_NOT_FOUND";
    }

    status = "OK";

    // READ FILE CONTENTS AND WRITE TO CLIENT
    fd = open(fileName, O_RDONLY | O_SYNC);
    if (fd == -1) {
        fprintf(stderr, "Failed to open file '%s': %s\n", fileName, strerror(errno));
        status = "FILE_NOT_FOUND";
    }
    status = "OK";
    printf("Opened file name sent by client\n");

    // Get total file size
    fseek(fileName, 0L, SEEK_END);
    fileSize = ftell(fileName);
    rewind(fileName);

    // construct header
    snprintf(header, sizeof(header), "GETFILE %s %ld\r\n\r\n", status, fileSize);

    // write header
    if (write(cfd, header, sizeof(header)) == -1) {
        fprintf(stderr, "Could not write the header back to client socket %s\n", strerror(errno));
        return -1;
    }

    // write file data in chunks
    while ((fileBytesRead = read(fd, buf, BUFSIZE)) > 0) {
        printf("Read %zd bytes from file\n", fileBytesRead);
        if (write(cfd, buf, fileBytesRead) == -1) {
            fprintf(stderr, "Could not write file contents back to client socket %s\n", strerror(errno));
            return -1;
        }
        printf("Wrote %zd bytes to client\n", fileBytesRead);
    }
    printf("File reading loop finished with status: %zd\n", fileBytesRead);

    if (fileBytesRead == -1) {
        fprintf(stderr, "Error reading from file: %s\n", strerror(errno));
        return -1;
    }

    return fd;
}

int main(int argc, char *argv[]) {
    int cfd;
    socklen_t addrlen;              // Length of client address structure
    struct sockaddr_storage claddr; // Client address structure (IPv4 or IPv6)


    int lfd = setup_socket(argv[1]);

    while (1) {
        
        addrlen = sizeof(struct sockaddr_storage);
        // Accept a client connection
        cfd = accept(lfd, (struct sockaddr *) &claddr, &addrlen);
        if (cfd == -1) {
            fprintf(stderr, "Failed to accept connection %s\n", strerror(errno));
            return -1;
        }

        printf("Accepted connection from client\n");

        // READ FILE PATH FROM CLIENT

        int fd = handle_file_transfer(cfd);

        if (fd == -1) 
            return -1;

        
        if (close(cfd) == -1) {
            fprintf(stderr, "Could not close client socket %s\n", strerror(errno));
            return -1;
        }
        printf("Closed the client socket\n");

        if (close(fd) == -1) {
            fprintf(stderr, "Could not close file %s\n", strerror(errno));
            return -1;
        }    
        printf("Closed the file descriptor\n");
    }
    

    if (close(lfd) == -1) {
        fprintf(stderr, "Could not close listening socket %s\n", strerror(errno));
        return -1;
    }
    printf("Closed the listening socket\n");

    return 0;
}
    