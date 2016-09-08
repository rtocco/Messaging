#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define PORT "9999"
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
    socklen_t socketSize;
    struct addrinfo info;
    struct addrinfo *serverInfo;
    struct addrinfo *n;
    struct sockaddr_storage clientAddress;

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

        printf("Connected\n");
        char *message = "Hello there.\n";
        send(takenSock, message, strlen(message), 0);

    } else if(!strcmp(option, "-c") && argc == 3) { // run as the client
        status = getaddrinfo(remoteAddress, PORT, &info, &serverInfo);
        sock = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
        connect(sock, serverInfo->ai_addr, serverInfo->ai_addrlen);
        char *buffer;
        printf("Connected\n");
        recv(sock, buffer, 100, 0);
        printf("%s\n", buffer);
    } else {
        printf("Improper input\n");
    }


    return 0;
}
