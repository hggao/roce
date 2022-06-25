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

/*
 * Listening on specified port and wait for client to connect.
 *
 * Note:
 *      The current implemementation accept only one connection.
 *      Once the client connnect accepted, we'll close the listener.
 */
int accepting_client(struct user_parameter *uparam)
{
    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;

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
    servaddr.sin_port = htons(uparam->server_port);
    if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        return -2;
    }
    printf("Socket successfully binded at *:%d\n", uparam->server_port);

    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        return -3;
    }
	printf("Server listening..\n");

    // Accept the data packet from client and verification
    len = sizeof(cli);
    connfd = accept(sockfd, (struct sockaddr*)&cli, &len);
    if (connfd < 0) {
        printf("server accept failed...\n");
        return -4;
    }
	printf("server accept the client...\n");

    close(sockfd);
    return connfd;
}

int main(int argc, char *argv[])
{
    int connfd;
    struct user_parameter uparam;

    if (parse_parameters(argc, argv, &uparam, 1) != 0) {
        print_usage(argv[0]);
        return 1;
    }

    printf("Hello there! This is RoCE Server App\n");

    connfd = accepting_client(&uparam);
    if (connfd < 0) {
        return 2;
    }

    connection_worker(connfd);

    close(connfd);

    return 0;
}
