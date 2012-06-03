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
#include "client.h"
#include "message.h"
#include "commands.h"

// TODO: Configuration structure, store stuff like this there at startup
#define PORT 6667

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
    
    info_print("Registering event handlers");
    event_register_handlers();
    info_print("Registering command handlers");
    command_register_handlers();
    
    info_print("Entering main event loop");
    event_start_loop(serverfd);
    
    return 0;
}

