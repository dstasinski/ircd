#ifndef EVENT_H
#define	EVENT_H

#include "socket.h"


#define MAXEVENTS 32
#define TIMEOUT 5000

typedef enum event_flags
{
    event_flags_timeout         = 1,
    event_flags_connect         = 2,
    event_flags_disconnect      = 4,
    event_flags_data            = 8,
} event_flags;

typedef struct event_callback_data
{
    event_flags flags;
    const socket_event_data *event_data;
    const socket_event_data *connected_event_data;
    const char *buffer;
    ssize_t buffer_length;
} event_callback_data;

typedef int (*event_callback_func)(event_callback_data *callback_data);

typedef struct event_handler 
{
    event_callback_func callback;
    event_flags flags;
    struct event_handler *next;
} event_handler;

extern event_handler *event_handlers;

void event_register_handler(event_callback_func callback, event_flags flags);
//TODO: deregister all
void event_dispatch_event(event_flags flags, event_callback_data *callback_data);

void event_start_loop(int serverfd, int epollfd);

#endif	/* EVENT_H */