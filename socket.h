#ifndef SOCKET_H
#define	SOCKET_H

#include "client.h"

typedef struct socket_event_data
{
    int fd;
    client_data *client;
} socket_event_data;

int socket_create_and_bind(unsigned long addr, unsigned short port);
int socket_set_nonblocking(int socketfd);
int socket_listen(int socketfd, int backlog);

int socket_epoll_ctl(int socketfd, int epollfd, client_data *client, socket_event_data **event_data);
int socket_epoll_create_and_setup(int socketfd);
#endif	/* SOCKET_H */
