#ifndef __ROCE_H__
#define __ROCE_H__

#include <infiniband/verbs.h>

#define DEFAULT_SERVER_IP   "127.0.0.1"
#define DEFAULT_SERVER_PORT 2022

#define MAX_PACKET_LEN 1024

#define DEFAULT_DEV_NAME   "bnxt_re0"

struct user_parameter {
    int interactive_mode;   //0: non-interactive mode, 1: interactive mode
    int is_server;          //0: client, 1: server
    char server_ip[16];     //Server IPv4 address
    int server_port;        //TCP listening port for management connection

    char devname[16];
};

int send_data(int fd, char *buff, int len);
int recv_data(int ffd, char *buff);

int parse_parameters(int argc, char *argv[], struct user_parameter *param, int is_svr);
int print_usage(char *appname);

struct ibv_context* open_roce_dev(struct user_parameter *param);
void close_roce_dev(struct ibv_context *ibv_ctx);

#endif //__ROCE_H__
