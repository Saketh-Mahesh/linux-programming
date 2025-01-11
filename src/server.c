#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main()
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    printf("Socket created\n");
    return 0;
}
