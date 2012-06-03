#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>

#include "common.h"
#include "socket.h"
#include "event.h"
#include "util.h"
#include "client.h"

event_handler *event_handlers = NULL;

void event_register_handler(event_flags flags, event_callback_func callback)
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
            // TODO: get return value, maybe dispatch error event if result != 0
            handler->callback(callback_data);
        }
        handler = handler->next;
    }
}

void event_register_handlers()
{
    event_register_handler(event_flags_data_in, client_callback_data_in);
    event_register_handler(event_flags_data_out, send_callback_data_out);
}

void event_start_loop_epoll(int serverfd);
void event_start_loop_select(int serverfd);

void event_start_loop(int serverfd)
{
#ifdef USE_EPOLL
    event_start_loop_epoll(serverfd);
#else
    event_start_loop_select(serverfd);
#endif
}

int event_read_available(client_data *client_event_data, event_callback_data *callback_data)
{
    ssize_t read_size;
    char buffer[512];
    
    int disconnect = 0;
    
    while (1)
    {
        read_size = read(client_event_data->fd, buffer, sizeof(buffer));
        if (read_size < 0)
        {
            // EAGAIN or EWOULDBLOCK means all available data 
            // has been read, everything else is an error
            // in which case we disconnect this client
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                // Ignore the error if the client sent QUIT
                // (or is quitting for some other reason)
                if (client_event_data->quitting == 0)
                {
                    error_print("read");
                }
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
            if (read_size == 1 && buffer[0] == 0x04)
            {
                /* Ctrl-D = EOT (0x04, End of Transmission) */
                disconnect = 1;
                break;
            }

            buffer[read_size] = '\0';

            callback_data->buffer = buffer;
            callback_data->buffer_length = read_size;
            event_dispatch_event(event_flags_data_in, callback_data);
        }
    }
    
    if (disconnect)
    {
        event_disconnect_client(client_event_data, callback_data);
        
        return -1;
    }
    
    return 0;    
}

void event_disconnect_client(client_data *client_event_data, event_callback_data *callback_data)
{
    info_print_format("Closing connection, fd: %d", client_event_data->fd);
    close(client_event_data->fd);

    event_dispatch_event(event_flags_disconnect, callback_data);
    client_delete(client_event_data);
}