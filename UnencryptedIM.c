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
#define BUFFER 10 // Up to 10 connections can be in queue.

int main(int argc, char **argv) {
    if(argc != 2 && argc != 3) {
        return 1;
    }

    char *option = argv[1];
    char *remoteAddress;
    if(argc == 3) {
        remoteAddress = argv[2];
    }

    int status, // Checks if anything has gone wrong.
        sock, // Socket file descriptor.
        takenSock; // Socket file descriptor for when client has been accepted.
    char buffer[101]; // For recieving messages.
    socklen_t socketSize;
    struct addrinfo info; // To be passed into getaddrinfo to get server information.
    struct addrinfo *serverInfo;
    struct sockaddr_storage clientAddress;

    fd_set set; // Used by select in both the client and the server.

    memset(&info, 0, sizeof info);
    info.ai_family = AF_UNSPEC; // ipv4 or ipv6
    info.ai_socktype = SOCK_STREAM; // Stream, not Datagram.

    if(!strcmp(option, "-s")) { // Run as the server.
        info.ai_flags = AI_PASSIVE; // Local address info.
        status = getaddrinfo(NULL, PORT, &info, &serverInfo);
        sock = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
        bind(sock, serverInfo->ai_addr, serverInfo->ai_addrlen);
        listen(sock, BUFFER);
        socketSize = sizeof clientAddress;
        takenSock = accept(sock, (struct sockaddr *)&clientAddress, &socketSize);

        char message[100];
        while(1) {
            FD_ZERO (&set);
            FD_SET (0, &set); // stdin
            FD_SET (takenSock, &set); // socket
            status = select(FD_SETSIZE, &set, NULL, NULL, NULL); // Wait for input.
            if(status != 1) { break; }
            if(FD_ISSET(0, &set)) { // stdin
                memset(message, 0, sizeof message);
                read(STDIN_FILENO, message, 100);
                if(message[0] == '\0') { exit(1); } // Ctrl-D pressed, exit program.
                send(takenSock, message, strlen(message), 0);
            }
            if(FD_ISSET(takenSock, &set)) { // socket
                memset(buffer, 0, sizeof buffer);
                recv(takenSock, buffer, 100, 0);
                printf("%s", buffer);
            }
        }
        freeaddrinfo(serverInfo);

    } else if(!strcmp(option, "-c") && argc == 3) { // Run as the client.
        status = getaddrinfo(remoteAddress, PORT, &info, &serverInfo);
        sock = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
        connect(sock, serverInfo->ai_addr, serverInfo->ai_addrlen);

        char message[100];
        while(1) {
            FD_ZERO (&set);
            FD_SET (0, &set); // stdin
            FD_SET (sock, &set); // socket
            status = select(FD_SETSIZE, &set, NULL, NULL, NULL);
            if(status != 1) { break; }
            if(FD_ISSET(0, &set)) { // stdin
                memset(message, 0, sizeof message);
                read(STDIN_FILENO, message, 100);
                if(message[0] == '\0') { exit(1); } // Ctrl-D pressed, exit program.
                send(sock, message, strlen(message), 0);
            }
            if(FD_ISSET(sock, &set)) { // socket
                memset(buffer, 0, sizeof buffer);
                recv(sock, buffer, 100, 0);
                printf("%s", buffer);
            }
        }
        freeaddrinfo(serverInfo);
    }

    return 0;
}
