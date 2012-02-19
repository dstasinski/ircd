#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <errno.h>

#include "socket.h"
#include "util.h"

// TODO: Configuration structure, store stuff like this there at startup
#define MAXEVENTS 32
#define TIMEOUT 5000
#define PORT 6667

int main(int argc, char** argv) {    
    
    info_print_format("Starting server at port %d", PORT);
    int serverfd = socket_create_and_bind(INADDR_ANY, PORT);
    if (serverfd < 0)
    {
        error_print_exit("socket_create_and_bind");
    }
    
    info_print("Setting server socket to nonblocking mode");
    if (socket_set_nonblocking(serverfd) < 0)
    {
        error_print_exit("socket_set_nonblocking");
    }
    
    info_print("Listening with max number of clients");
    if (socket_listen(serverfd, -1) < 0)
    {
        error_print_exit("socket_listen");
    }
    
    info_print("Creating and setting up epolll descriptor");
    int epollfd = socket_epoll_create_and_setup(serverfd);
    if (epollfd < 0)
    {
        error_print_exit("socket_epoll_create_and_setup");
    }
    
    struct epoll_event *events = calloc(MAXEVENTS, sizeof(struct epoll_event));
    
    info_print("Entering main listen loop");
    while (1)
    {
        int n = epoll_wait(epollfd, events, MAXEVENTS, TIMEOUT);
        if (n < 0)
        {
            error_print_exit("epoll_wait");
        }
        
        if (n == 0)
        {
            /* No events, timed-out */
            printf(".");
            fflush(stdout);
        }
        else
        {
            for(int i = 0; i < n; i++)
            {
                if (events[i].events & EPOLLERR || events[i].events & EPOLLHUP || !(events[i].events & EPOLLIN))
                {
                    /* An error occurred on this socket (or disconnected) */
                    info_print_format("Socket error at event %d, fd: %d", i, events[i].data.fd);
                    close(events[i].data.fd);
                }
                else if (events[i].data.fd == serverfd)
                {
                    /* Event at the server descriptor - new incoming connection(s) */
                    while (1)
                    {
                        struct sockaddr client_addr;
                        socklen_t client_addr_len = sizeof(client_addr);
                        
                        int clientfd = accept(serverfd, &client_addr, &client_addr_len);
                        if (clientfd < 0)
                        {
                            if (errno == EAGAIN || errno == EWOULDBLOCK)
                            {
                                /* All waiting incoming connections have been processed */
                                break;
                            }
                            error_print("accept");
                            break;
                        }
                        
                        if (socket_set_nonblocking(clientfd) < 0)
                        {
                            error_print_exit("socket_set_nonblocking");
                        }
                        
                        
                        if (socket_epoll_ctl(clientfd, epollfd) < 0)
                        {
                            error_print_exit("socket_epoll_ctl");
                        }
                        
                        info_print_format("Accepted new connection, fd: %d", clientfd);
                    }
                }
                else
                {
                    /* Data available at this socket */
                    ssize_t read_size;
                    char buffer[1024];
                    int disconnect = 0;
                    while (1)
                    {
                        read_size = read(events[i].data.fd, buffer, sizeof(buffer));
                        if (read_size < 0)
                        {
                            /* EAGAIN means all available data has been read, 
                             * everything else is an error
                             */
                            if (errno != EAGAIN)
                            {
                                error_print("read");
                                disconnect = 1;
                            }
                            break;
                        }
                        
                        if (read_size == 0)
                        {
                            /* EOF, connection was closed */
                            disconnect = 1;
                            break;
                        }
                        else
                        {
                            // TODO: Proper fix for this
                            if (read_size == 1 && buffer[0] == 0x04)
                            {
                                /* Ctrl-D = EOT (End of Transmission) */
                                disconnect = 1;
                                break;
                            }
                            
                            /* Send buffer contents back to client */
                            if (write(events[i].data.fd, buffer, read_size) < 0)
                            {
                                error_print_exit("write");
                            }
                        }
                    }
                    if (disconnect)
                    {
                        info_print_format("Closing connection, fd: %d", events[i].data.fd);
                        close(events[i].data.fd);
                    }
                }
            }
        }
    }
    
    return 0;
}

