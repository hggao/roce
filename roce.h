#ifndef __ROCE_H__
#define __ROCE_H__

#define DEFAULT_SERVER_PORT 2022

#define MAX_PACKET_LEN 1024

int send_data(int fd, char *buff, int len);
int recv_data(int ffd, char *buff);


#endif //__ROCE_H__
