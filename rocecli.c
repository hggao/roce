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

int connect_server(struct user_parameter *uparam)
{
    int fd;
    struct sockaddr_in servaddr;

    // socket create and verification
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        printf("socket creation failed...\n");
        return -1;
    }
    printf("Socket successfully created..\n");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(DEFAULT_SERVER_PORT);
    if (inet_pton(AF_INET, uparam->server_ip, &servaddr.sin_addr) <= 0) {
        printf("inet_pton error occured\n");
        return -2;
    }
    // connect the client socket to server socket
    if (connect(fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        return -3;
    }
    printf("connected to the server..\n");
    return fd;
}

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

int client_run_tasks(int sockfd, struct user_parameter *param)
{
    char *msg = "This is the only message to transfer";
	char buff[MAX_PACKET_LEN + 1];
    int n;

    send_data(sockfd, msg, strlen(msg));
    printf("Sent Server : %s\n", msg);

	bzero(buff, sizeof(buff));
	n = recv_data(sockfd, buff);
	printf("From Server : %s\n", buff);

    struct ibv_context *ibv_ctx;
    ibv_ctx = open_roce_dev(param);

	printf("Pending doing something here......\n");

    close_roce_dev(ibv_ctx);

    return 0;
}

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in servaddr;
    struct user_parameter uparam;

    if (parse_parameters(argc, argv, &uparam, 0) != 0) {
        print_usage(argv[0]);
        return 1;
    }

    printf("Hello there! This is RoCE Client App\n");

    sockfd = connect_server(&uparam);
    if (sockfd < 0) {
        return 2;
    }

    if (uparam.interactive_mode) {
        client_interact(sockfd);
    } else {
        client_run_tasks(sockfd, &uparam);
    }

    // close the socket
    close(sockfd);

    printf("Goodbye!\n");
    return 0;
}
