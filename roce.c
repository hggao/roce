/*
 * Routines shared by RoCE Server and Client
 */
#include <errno.h>
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "roce.h"

int send_data(int fd, char *buff, int len)
{
    unsigned int pkg_len = len;

    write(fd, &pkg_len, sizeof(pkg_len));
    write(fd, buff, pkg_len);
}

int recv_data(int fd, char *buff)
{
    unsigned int pkg_len;
    int n, idx;

    pkg_len = 0;
    n = read(fd, &pkg_len, sizeof(pkg_len));
    if (n == 0 || pkg_len == 0) {
        return 0;
    }
    if (n != sizeof(pkg_len) || pkg_len > MAX_PACKET_LEN) {
        printf("Error cannot handle, quit. n = %d, len = %u\n", n, pkg_len);
        return -1;
    }

    idx = 0;
    while (idx < pkg_len) {
        n = read(fd, buff + idx, pkg_len - idx);
        if (n <= 0) {
            printf("Error read data on socket, n = %d, idx = %d\n", n, idx);
            return -2;
        }
        idx += n;
    }
    return idx;
}

int parse_parameters(int argc, char *argv[], struct user_parameter *param, int is_svr)
{
    int i;

    //Init params to default
    param->interactive_mode = 0;
    param->is_server = is_svr;
    strcpy(param->server_ip, DEFAULT_SERVER_IP);
    param->server_port = DEFAULT_SERVER_PORT;
    strcpy(param->devname, DEFAULT_DEV_NAME);

    //Parsing parameters
    for (i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            if (is_svr) {
                return 2;
            }
            if (strlen(argv[i]) < 16) {
                strcpy(param->server_ip, argv[i]);
            } else {
                printf("Invalid IP, ignored.\n");
            }
        } else if (strcmp(argv[i], "-h") == 0) {
            return 1;
        } else if (strcmp(argv[i], "-i") == 0) {
            param->interactive_mode = 1;
        } else if (strcmp(argv[i], "-p") == 0) {
            i++;
            if (i < argc) {
                param->server_port = atoi(argv[i]);
            }
        } else if (strcmp(argv[i], "-d") == 0) {
            i++;
            if (i < argc && strlen(argv[i]) < 16) {
                strcpy(param->devname, argv[i]);
            } else {
                printf("Invalid device name, ignored.\n");
            }
        }
    }
    //Print parameters

    return 0;
}

int print_usage(char *appname)
{
    printf("\nUsage:\n"
           "%s [server ip] [-i] [-p <port>]\n", appname);
    printf("        -h           print this usage information\n"
           "        -i           interactive mode, default is non-interactive mode\n"
           "        -p <port>    Server TCP listening port for management connectoin.\n"
           "                     default is 2022\n"
           "        [server ip]  IPv4 address of server. Only need to provide for rocecli\n"
           "                     if not provided for rocesvr, default is 127.0.0.1\n"
           "        -d <devname> RoCE device to be used, specified by devname.\n"
           "                     if not provided, default is bnxt_re0\n"
          );
}

int open_roce_dev(struct user_parameter *param, struct roce_context *rocectx)
{
    int num_of_device = 0;
    struct ibv_device **dev_list;
    struct ibv_device *ib_dev = NULL;
    struct ibv_context *context = NULL;
    int rc = 0;

    dev_list = ibv_get_device_list(&num_of_device);
    if (num_of_device <= 0) {
        printf(" Did not detect RoCE devices.\n");
        return -1;
    }
    for (; (ib_dev = *dev_list); ++dev_list) {
        if (strcmp(ibv_get_device_name(ib_dev), param->devname) == 0) {
            break;
        }
    }
    if (!ib_dev) {
        printf("RoCE device [%s] not found.\n", param->devname);
        rc = -2;
        goto not_found;
    }

    printf("RoCE device [%s](%s) found.\n", ib_dev->name, ib_dev->dev_name);
    printf("    node_type      : %d\n", (int)ib_dev->node_type);
    printf("    transport_type : %d\n", (int)ib_dev->transport_type);
    printf("    IB name        : %s\n", ib_dev->name);
    printf("    dev_name       : %s\n", ib_dev->dev_name);
    printf("    dev_path       : %s\n", ib_dev->dev_path);
    printf("    ibdev_path     : %s\n", ib_dev->ibdev_path);
    
    rocectx->ctx = ibv_open_device(ib_dev);
    if (!rocectx->ctx) {
        rc = -3;
        printf("Couldn't open device.\n");
    } else {
        printf("Device opened:\n");
        printf("    cmd_fd   : %d\n", rocectx->ctx->cmd_fd);
        printf("    async_fd : %d\n", rocectx->ctx->async_fd);
    }

not_found:
    ibv_free_device_list(dev_list);
    
    return rc;
}

void close_roce_dev(struct roce_context *rocectx)
{
    ibv_close_device(rocectx->ctx);
    printf("RoCE device [%s] closed.\n", rocectx->ctx->device->name);
    rocectx->ctx = NULL;
}

int roce_send(struct roce_context *rocectx)
{
    int rc = 0;

    // Protect domain alloc
    rocectx->pd = ibv_alloc_pd(rocectx->ctx);
    if (!rocectx->pd) {
        printf("Allocate protect domain failed.\n");
        return -1;
    }
    printf("Protect domain allocated.\n");

    // Allocate buffer
    rocectx->buf = memalign(MR_ALIGNMENT, MR_FULL_SIZE);
    if (rocectx->buf == NULL) {
        printf("Allocate memory buffer failed.\n");
        rc = -2;
        goto mem_failed;
    }
    rocectx->buf_len = MR_FULL_SIZE;
    rocectx->sbuf = rocectx->buf;
    rocectx->sbuf_len = MR_HALF_SIZE;
    rocectx->rbuf = (void*)((char*)rocectx->buf + MR_HALF_SIZE);
    rocectx->rbuf_len = MR_HALF_SIZE;
    printf("Memory buffer allocated.\n");

    //Memory region register
    rocectx->mr = ibv_reg_mr(rocectx->pd, rocectx->buf, rocectx->buf_len,
                             IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE |
                             IBV_ACCESS_REMOTE_READ);
    if (rocectx->mr == NULL) {
        printf("Register memory region failed.\n");
        rc = -3;
        goto mr_failed;
    }
    printf("Memory region registered.\n");

    //Create scq
    rocectx->scq = ibv_create_cq(rocectx->ctx, NUM_CQE, NULL, NULL, 0);
    if (rocectx->scq == NULL) {
        printf("Create send completion queue failed. errno=%d\n", errno);
        rc = -4;
        goto scq_failed;
    }
    printf("Send completion queue created.\n");

    //Create rcq
    rocectx->rcq = ibv_create_cq(rocectx->ctx, NUM_CQE, NULL, NULL, 0);
    if (rocectx->rcq == NULL) {
        printf("Create receive completion queue failed.\n");
        rc = -4;
        goto rcq_failed;
    }
    printf("Receive completion queue created.\n");

    //Create QP
    struct ibv_qp_init_attr init_attr;
    init_attr.qp_context = NULL;
    init_attr.send_cq = rocectx->scq;
    init_attr.recv_cq = rocectx->rcq;
    init_attr.srq = NULL;
    init_attr.cap.max_send_wr = 6;
    init_attr.cap.max_recv_wr = 6;
    init_attr.cap.max_send_sge = 6;
    init_attr.cap.max_recv_sge = 6;
	init_attr.cap.max_inline_data = 96;
    init_attr.qp_type = IBV_QPT_RC;
    init_attr.sq_sig_all = 1;
    rocectx->qp = ibv_create_qp(rocectx->pd, &init_attr);
    if (rocectx->qp == NULL) {
        printf("Create QP failed.\n");
        rc = -5;
        goto qp_failed;
    }
    printf("QP created.\n");



//-----------------Pivot----------------------------


    //QP destroy
    if (ibv_destroy_qp(rocectx->qp) != 0) {
        printf("Destroy qp failed.\n");
        rc--;
    } else {
        rocectx->qp = NULL;
        printf("QP destroyed.\n");
    }

qp_failed:
    //Delete rcq
    if (ibv_destroy_cq(rocectx->rcq) != 0) {
        printf("Destroy rcq failed.\n");
        rc--;
    } else {
        rocectx->rcq = NULL;
        printf("Receive completion queue destroyed.\n");
    }
rcq_failed:
    //Delete scq
    if (ibv_destroy_cq(rocectx->scq) != 0) {
        printf("Destroy scq failed.\n");
        rc--;
    } else {
        rocectx->scq = NULL;
        printf("Send completion queue destroyed.\n");
    }

scq_failed:
    //Memory region unregister
    if (ibv_dereg_mr(rocectx->mr) != 0) {
        printf("Unregister memory region failed.\n");
        rc--;
    } else {
        rocectx->mr = NULL;
        printf("Memory region unregistered.\n");
    }

mr_failed:
    // Free memory buffer
    free(rocectx->buf);
    rocectx->buf = NULL;
    rocectx->sbuf = NULL;
    rocectx->rbuf = NULL;

mem_failed:
    if (ibv_dealloc_pd(rocectx->pd) != 0) {
        printf("Deallocate protect domain failed.\n");
        rc--;
    } else {
        rocectx->pd = NULL;
        printf("Protect domain deallocated.\n");
    }

    return rc;
}
