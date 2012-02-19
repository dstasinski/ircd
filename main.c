#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include "socket.h"
#include "util.h"
#include "event.h"

// TODO: Configuration structure, store stuff like this there at startup
#define PORT 6667

int callback_timeout(event_callback_data *callback_data) 
{
    printf(".");
    fflush(stdout);
    return 0;
}

int callback_connect(event_callback_data *callback_data)
{
    info_print_format("Client connected, fd=%d", callback_data->connected_event_data->fd);
    callback_data->connected_event_data->client->data = 100+callback_data->connected_event_data->fd;
    
    char buffer[100];
    int length = sprintf(buffer, "Welcome client %d!\r\nPlease enter your name:\r\n", callback_data->connected_event_data->fd);
    if (write(callback_data->connected_event_data->fd, buffer, length) < 0)
    {
        error_print_exit("write");
    }
    
    return 0;
}

int callback_recv(event_callback_data *callback_data)
{
    /*info_print_format("%d", callback_data->event_data->client->data);
    callback_data->event_data->client->data += 1;*/
    
    /* Send buffer contents back to client */
    //if (write(callback_data->event_data->fd, callback_data->buffer, callback_data->buffer_length) < 0)
    //{
    //    error_print_exit("write");
    //}
    
    if (callback_data->event_data->client->name == NULL)
    {
        callback_data->event_data->client->name = malloc((callback_data->buffer_length-1)*sizeof(char));
        memcpy(callback_data->event_data->client->name, callback_data->buffer, callback_data->buffer_length-2);
        callback_data->event_data->client->name[callback_data->buffer_length-2] = '\0';
        
        char buffer[100];
        int length = sprintf(buffer, "Welcome %s\r\n", callback_data->event_data->client->name);
        write(callback_data->event_data->fd, buffer, length);
    }
    else
    {
        info_print_format("Message from %s: %s", callback_data->event_data->client->name, callback_data->buffer);
    }
    
    return 0;
}

int callback_disconnect(event_callback_data *callback_data)
{
    info_print_format("Client disconnected, fd=%d, name=%s", callback_data->event_data->fd, (callback_data->event_data->client->name != NULL) ? callback_data->event_data->client->name : "NULL");
    
    return 0;
}

int main(int argc, char** argv)
{    
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
    
    info_print("Registering event handlers");
    event_register_handler(callback_timeout, event_flags_timeout);
    event_register_handler(callback_connect, event_flags_connect);
    event_register_handler(callback_recv, event_flags_data);
    
    info_print("Entering main event loop");
    event_start_loop(serverfd, epollfd);
    
    return 0;
}

