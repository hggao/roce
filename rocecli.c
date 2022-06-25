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

void client_interact(void)
{
/*
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
*/
    getchar();
}

int client_run_tasks(struct user_parameter *param)
{
    struct roce_context rocectx;

    rocectx.sockfd = connect_server(param);
    if (rocectx.sockfd < 0) {
        return 2;
    }

    if (open_roce_dev(param, &rocectx) != 0) {
        return -1;
    }

    if (roce_send(&rocectx) != 0) {
        printf("Error perform roce send.\n");
    }

    close_roce_dev(&rocectx);

    close(rocectx.sockfd);

    return 0;
}

int main(int argc, char *argv[])
{
    struct user_parameter uparam;

    if (parse_parameters(argc, argv, &uparam, 0) != 0) {
        print_usage(argv[0]);
        return 1;
    }

    printf("Hello there! This is RoCE Client App\n");

    if (uparam.interactive_mode) {
        client_interact();
    } else {
        client_run_tasks(&uparam);
    }

    printf("Goodbye!\n");
    return 0;
}
