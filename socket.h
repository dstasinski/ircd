#ifndef SOCKET_H
#define	SOCKET_H

int socket_create_and_bind(unsigned long addr, unsigned short port);
int socket_set_nonblocking(int socketfd);
int socket_listen(int socketfd, int backlog);

int socket_epoll_create_and_setup(int socketfd);
#endif	/* SOCKET_H */
