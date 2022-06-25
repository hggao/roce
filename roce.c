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

    //Parsing parameters
    for (i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            if (is_svr) {
                return 2;
            } else {
                strcpy(param->server_ip, argv[i]);
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
        }
    }
    //Print parameters

    return 0;
}

int print_usage(char *appname)
{
    printf("\nUsage:\n"
           "%s [server ip] [-i] [-p <port>]\n", appname);
    printf("        -h          print this usage information\n"
           "        -i          interactive mode, default is non-interactive mode\n"
           "        -p <port>   Server TCP listening port for management connectoin.\n"
           "                    default is 2022\n"
           "        [server ip] IPv4 address of server. Only need to provide for rocecli\n"
           "                    if not provided for rocesvr, default is 127.0.0.1\n"
          );
}
