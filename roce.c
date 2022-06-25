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
