/*
 * This is RoCE Client
 */
#include <stdio.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "roce.h"

void client_interact(int sockfd)
{
	char buff[MAX_PACKET_LEN + 1];
	int n;

	for (;;) {
		bzero(buff, sizeof(buff));
		printf("rocecli>");
		n = 0;
		while (n < MAX_PACKET_LEN && (buff[n++] = getchar()) != '\n') ;
        n--;
        buff[n] = '\0';
        if (n == 0) {
		    printf("Stop now.\n");
            break;
        }
        send_data(sockfd, buff, n);
		printf("Sent Server : %s\n", buff);

		bzero(buff, sizeof(buff));
		n = recv_data(sockfd, buff);
		printf("From Server : %s\n", buff);
	}
}

int main(int argc, char *argv[])
{
    int sockfd, connfd;
    struct sockaddr_in servaddr;

    printf("Hello there! This is RoCE Client App\n");
    if (argc != 2) {
        printf("\n Usage: %s <ip of server> \n",argv[0]);
        return 1;
    }

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        return -1;
    }
    printf("Socket successfully created..\n");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(DEFAULT_SERVER_PORT);
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        printf("inet_pton error occured\n");
        return 1;
    }
    // connect the client socket to server socket
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        return -2;
    }
    printf("connected to the server..\n");

    // function for chat
    client_interact(sockfd);

    // close the socket
    close(sockfd);

    printf("Goodbye!\n");
    return 0;
}
