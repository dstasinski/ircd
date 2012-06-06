#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include "signal.h"
#include "socket.h"
#include "util.h"
#include "event.h"
#include "client.h"
#include "message.h"
#include "common.h"
#include "commands.h"

int main(int argc, char** argv)
{    
    
    unsigned short port = LISTEN_PORT;
    unsigned long addr = LISTEN_ADDRESS;
    
    /* Command line options */
#if _POSIX_C_SOURCE >= 2 || _XOPEN_SOURCE
    int opt;
    while ((opt = getopt(argc, argv, "va:p:")) != -1)
    {
        switch(opt)
        {
            case 'v': /* Verbose */
                debug_set_verbose(1);
                break;
            case 'a': /* Address */
                printf("a:[%s]", optarg);
                break;
            case 'p':
                break;
            case '?':
                fprintf(stderr, "%s: invalid option -- '%c'\n", argv[0], optopt);
                fprintf(stderr, "Usage: %s [-v] [-a addr] [-p port]\n", argv[0]);
                exit(EXIT_FAILURE);
            default:
                error_print_exit("getopt");
        }
    }
#else
#warning "getopt not available. Will use default values."
#warning "getopt requires _XOPEN_SOURCE or _POSIX_C_SOURCE >= 2"
#endif
    
    info_print_format("Starting server at port %d", port);
    int serverfd = socket_create_and_bind(addr, port);
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
    
    info_print("Registering signal handlers");
    signal_register_handlers();
    
    info_print("Entering main event loop");
    event_start_loop(serverfd);
    
    info_print("Shutdown complete");
    return EXIT_SUCCESS;
}

