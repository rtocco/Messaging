#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT "9998"
#define BUFFER 10

int main(int argc, char **argv) {
    if(argc != 2 && argc != 3) {
        printf("Improper number of arguments\n");
        return 1;
    }

    char *option = argv[1];
    char *remoteAddress;
    if(argc == 3) {
        remoteAddress = argv[2];
    }

    int status;
    int sock;
    int takenSock;
    char buffer[101];
    socklen_t socketSize;
    struct addrinfo info;
    struct addrinfo *serverInfo;
    struct addrinfo *n;
    struct sockaddr_storage clientAddress;

    struct timeval timeout;
    timeout.tv_sec = 100;
    timeout.tv_usec = 0;
    fd_set set;

    memset(&info, 0, sizeof info);
    info.ai_family = AF_UNSPEC;
    info.ai_socktype = SOCK_STREAM;

    if(!strcmp(option, "-s")) { // run as the server
        info.ai_flags = AI_PASSIVE;
        status = getaddrinfo(NULL, PORT, &info, &serverInfo);
        sock = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
        bind(sock, serverInfo->ai_addr, serverInfo->ai_addrlen);
        listen(sock, BUFFER);
        printf("Listening on port %s\n", PORT);
        socketSize = sizeof clientAddress;
        takenSock = accept(sock, (struct sockaddr *)&clientAddress, &socketSize);

        char message[100];
        while(1) {
            FD_ZERO (&set);
            FD_SET (0, &set);
            FD_SET (takenSock, &set);
            status = select(FD_SETSIZE, &set, NULL, NULL, &timeout);
            printf("Past select\n");
            if(status != 1) { exit(1); }
            if(FD_ISSET(0, &set)) {
                memset(message, 0, sizeof message);
                read(STDIN_FILENO, message, 100);
                send(takenSock, message, strlen(message), 0);
            }
            if(FD_ISSET(takenSock, &set)) {
                memset(buffer, 0, sizeof buffer);
                recv(takenSock, buffer, 100, 0);
                printf("%s", buffer);
            }
        }
        freeaddrinfo(serverInfo);

    } else if(!strcmp(option, "-c") && argc == 3) { // run as the client
        status = getaddrinfo(remoteAddress, PORT, &info, &serverInfo);
        sock = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
        connect(sock, serverInfo->ai_addr, serverInfo->ai_addrlen);

        char message[100];
        while(1) {
            FD_ZERO (&set);
            FD_SET (0, &set);
            FD_SET (sock, &set);
            status = select(FD_SETSIZE, &set, NULL, NULL, &timeout);
            if(status != 1) { exit(1); }
            if(FD_ISSET(0, &set)) {
                memset(message, 0, sizeof message);
                read(STDIN_FILENO, message, 100);
                send(sock, message, strlen(message), 0);
            }
            if(FD_ISSET(sock, &set)) {
                memset(buffer, 0, sizeof buffer);
                recv(sock, buffer, 100, 0);
                printf("%s", buffer);
            }
        }
        freeaddrinfo(serverInfo);
    } else {
        printf("Improper input\n");
    }


    return 0;
}
