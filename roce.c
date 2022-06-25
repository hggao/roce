/*
 * Routines shared by RoCE Server and Client
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

struct ibv_context* open_roce_dev(struct user_parameter *param)
{
    int num_of_device = 0;
    struct ibv_device **dev_list;
    struct ibv_device *ib_dev = NULL;
    struct ibv_context *context = NULL;

    dev_list = ibv_get_device_list(&num_of_device);
    if (num_of_device <= 0) {
        printf(" Did not detect RoCE devices.\n");
        return NULL;
    }
    for (; (ib_dev = *dev_list); ++dev_list) {
        if (strcmp(ibv_get_device_name(ib_dev), param->devname) == 0) {
            break;
        }
    }
    if (!ib_dev) {
        printf("RoCE device [%s] not found.\n", param->devname);
        goto not_found;
    }

    printf("RoCE device [%s](%s) found.\n", ib_dev->name, ib_dev->dev_name);
    printf("    node_type      : %d\n", (int)ib_dev->node_type);
    printf("    transport_type : %d\n", (int)ib_dev->transport_type);
    printf("    IB name        : %s\n", ib_dev->name);
    printf("    dev_name       : %s\n", ib_dev->dev_name);
    printf("    dev_path       : %s\n", ib_dev->dev_path);
    printf("    ibdev_path     : %s\n", ib_dev->ibdev_path);
    
    context = ibv_open_device(ib_dev);
    if (!context) {
        printf("Couldn't open device.\n");
    } else {
        printf("Device opened:\n");
        printf("    cmd_fd   : %d\n", context->cmd_fd);
        printf("    async_fd : %d\n", context->async_fd);
    }

not_found:
    ibv_free_device_list(dev_list);
    
    return context;
}

void close_roce_dev(struct ibv_context *ibv_ctx)
{
    ibv_close_device(ibv_ctx);
    printf("RoCE device [%s] closed.\n", ibv_ctx->device->name);
}
