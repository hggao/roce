/*
 * This is RoCE Server
 */

#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "roce.h"

// Function designed for communicate between client and server.
void connection_worker(int connfd)
{
    char buff[MAX_PACKET_LEN + 1];
    int n, idx;

    // infinite loop for chat
    for (;;) {
        n = recv_data(connfd, buff);
        if (n <= 0) {
            printf("Client closed, we quit too.\n");
            break;
        }
	buff[n] = '\0';
        printf("From client: %d bytes - '%s', reply back the same\n", n, buff);

        // and send that buffer to client
        send_data(connfd, buff, n);
    }
}

int main(int argc, char *argv[])
{
    short listen_port = DEFAULT_SERVER_PORT;
    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;

    printf("Hello there! This is RoCE Server App\n");

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        return -1;
    }
    printf("Socket successfully created..\n");

    // Binding to given IP:port and verification
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(listen_port);
    if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        return -2;
    }
    printf("Socket successfully binded at *:%d\n", (int)listen_port);

    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        return -3;
    }
	printf("Server listening..\n");
    len = sizeof(cli);

    // Accept the data packet from client and verification
    connfd = accept(sockfd, (struct sockaddr*)&cli, &len);
    if (connfd < 0) {
        printf("server accept failed...\n");
        return -4;
    }
	printf("server accept the client...\n");

    // Function for chatting between client and server
    connection_worker(connfd);

    // After chatting close the socket
    close(sockfd);

    return 0;
}
