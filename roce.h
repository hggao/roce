#ifndef __ROCE_H__
#define __ROCE_H__

#include <infiniband/verbs.h>

#define DEFAULT_SERVER_IP   "127.0.0.1"
#define DEFAULT_SERVER_PORT 2022

#define MAX_PACKET_LEN 1024

#define DEFAULT_DEV_NAME   "bnxt_re0"

#define MR_ALIGNMENT    4096
#define MR_HALF_SIZE   (4096*8)
#define MR_FULL_SIZE   (MR_HALF_SIZE*2)

#define NUM_CQE		8

struct user_parameter {
    int interactive_mode;   //0: non-interactive mode, 1: interactive mode
    int is_server;          //0: client, 1: server
    char server_ip[16];     //Server IPv4 address
    int server_port;        //TCP listening port for management connection

    char devname[16];
};

struct roce_context {
    int sockfd;
    struct ibv_context *ctx;
    struct ibv_pd *pd;

    void *buf;
    size_t buf_len;
    struct ibv_mr *mr;
    void *sbuf;
    size_t sbuf_len;
    void *rbuf;
    size_t rbuf_len;

    struct ibv_cq *scq;
    struct ibv_cq *rcq;

    struct ibv_qp *qp;
};

int send_data(int fd, char *buff, int len);
int recv_data(int ffd, char *buff);

int parse_parameters(int argc, char *argv[], struct user_parameter *param, int is_svr);
int print_usage(char *appname);

int  open_roce_dev(struct user_parameter *param, struct roce_context *rocectx);
void close_roce_dev(struct roce_context *rocectx);
int  roce_send(struct roce_context *rocectx);

#endif //__ROCE_H__
