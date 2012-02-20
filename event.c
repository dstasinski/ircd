#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <errno.h>
#include <fcntl.h>

#include "socket.h"
#include "event.h"
#include "util.h"
#include "client.h"

event_handler *event_handlers = NULL;

void event_register_handler(event_callback_func callback, event_flags flags)
{
    event_handler *handler = malloc(sizeof(event_handler));
    handler->callback = callback;
    handler->flags = flags;
    handler->next = NULL;
    
    event_handler *last = event_handlers;
    if (last == NULL)
    {
        event_handlers = handler;
    }
    else
    {
        while (last->next != NULL)
        {
            last = last->next;
        }
        last->next = handler;
    }
}

void event_dispatch_event(event_flags flags, event_callback_data *callback_data)
{
    if (callback_data != NULL)
    {
        callback_data->flags = flags;
    }
    
    event_handler *handler = event_handlers;
    while (handler != NULL)
    {
        if ((handler->flags & flags) != 0)
        {
            handler->callback(callback_data);
        }
        handler = handler->next;
    }
}

void event_start_loop(int serverfd, int epollfd)
{
    struct epoll_event *events = calloc(MAXEVENTS, sizeof(struct epoll_event));
    
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
            event_dispatch_event(event_flags_timeout, NULL);
        }
        else
        {
            for(int i = 0; i < n; i++)
            {
                client_data *client_event_data = (client_data *) events[i].data.ptr;
                
                event_callback_data callback_data;
                callback_data.client = client_event_data;
                callback_data.client_new = NULL;
                callback_data.buffer = NULL;
                callback_data.buffer_length = 0;
                
                if (events[i].events & EPOLLERR || events[i].events & EPOLLHUP || !(events[i].events & EPOLLIN))
                {
                    /* An error occurred on this socket (or disconnected) */
                    info_print_format("Socket error at event %d, fd: %d", i, client_event_data->fd);
                    close(client_event_data->fd);
                }
                else if (client_event_data->fd == serverfd)
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
                        
                        client_data *client_new = client_allocate_new();
                        if (socket_epoll_ctl(clientfd, epollfd, client_new) < 0)
                        {
                            error_print_exit("socket_epoll_ctl");
                        }
                        
                        info_print_format("Accepted new connection, fd: %d", clientfd);
                        
                        callback_data.client_new = client_new;
                        event_dispatch_event(event_flags_connect, &callback_data);
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
                        read_size = read(client_event_data->fd, buffer, sizeof(buffer));
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
                            
                            // TODO: line splitting etc
                            buffer[read_size] = '\0';
                            
                            callback_data.buffer = buffer;
                            callback_data.buffer_length = read_size;
                            event_dispatch_event(event_flags_data, &callback_data);
                        }
                    }
                    if (disconnect)
                    {
                        // TODO: Reorder operations?
                        info_print_format("Closing connection, fd: %d", client_event_data->fd);
                        close(client_event_data->fd);
                        
                        event_dispatch_event(event_flags_disconnect, &callback_data);
                        client_delete(client_event_data);
                    }
                }
            }
        }
    }
}
