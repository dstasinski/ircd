/* 
 * File:   main.c
 * Author: lacop
 *
 * Created on February 18, 2012, 10:23 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <fcntl.h>

/****/
/* Copied from sockhelp.c (socket-faq-examples) */
void setnonblocking(int sockfd)
{
    int opts = fcntl(sockfd, F_GETFL);
    if (opts < 0)
    {
        perror("fcntl(F_GETFL)");
        exit(-1);
    }
    opts = opts | O_NONBLOCK;
    if (fcntl(sockfd, F_SETFL, opts) < 0)
    {
        perror("fcntl(F_SETFL)");
        exit(-1);
    }
}

int sock_write(int sockfd, char *buf, size_t count)
{
    size_t bytes_sent = 0;
    int this_write;
    
    while (bytes_sent < count)
    {
        do
        {
            this_write = write(sockfd, buf, count - bytes_sent);
        } while (this_write < 0 && errno == EINTR);
        if (this_write <= 0)
        {
            return this_write;
        }
        bytes_sent += this_write;
        buf += this_write;
    }
    return count;
} 

int sock_puts(int sockfd, char *str)
{
    sock_write(sockfd, str, strlen(str));
}

int sock_gets(int sockfd, char *str, size_t count)
{
    int bytes_read;
    int total_count = 0;
    char *current_position;
    char last_read = 0;
    
    current_position = str;
    while (last_read != 10)
    {
        bytes_read = read(sockfd, &last_read, 1);
        if (bytes_read <= 0)
        {
            /* The other side may have closed unexpectedly */
            return -1; /* Is this effective on other platforms than linux? */
        }
        if (total_count < count && last_read != 10 && last_read !=13)
        {
            current_position[0] = last_read;
            current_position++;
            total_count++;
        }
    }
    if (count > 0)
    {
        current_position[0] = 0;
    }
    return total_count;
}

/*****/

int main(int argc, char** argv) {
    printf("Launching server...\n");
     
    /* Create socket */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Socket error");
        return -1;
    }
    
    int reuse_addr = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));
    /* Set non-blocking */
    setnonblocking(sockfd);    
    
    /* Bind */
    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(6667);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind error");
        close(sockfd);
        return -1;
    }
    
    listen(sockfd, 5);
    printf("Listening for clients\n");
    
    int highest_fd = sockfd;
    int connections[5];
    memset((char *) &connections, 0, sizeof(connections));
    
    fd_set sockets;
    struct timeval timeout;
    
    while (1)
    {
        /* Reset sockets set, add the server socket */
        FD_ZERO(&sockets);
        FD_SET(sockfd, &sockets);
        /* Add other connections */
        for (int i = 0; i < 5; i++)
        {
            if (connections[i] != 0)
            {
                FD_SET(connections[i], &sockets);
                if (connections[i] > highest_fd)
                {
                    highest_fd = connections[i];
                }
            }
        }
        
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int readsocks = select(highest_fd+1, &sockets, (fd_set *) 0, (fd_set *) 0, &timeout);
        if (readsocks < 0) 
        {
            perror("Select error");
            return -1;
        }
        if (readsocks == 0)
        {
            /* No change, output heartbeat */
            printf("."); 
            fflush(stdout);
        }
        else
        {
            /* Something has happened */
            if (FD_ISSET(sockfd, &sockets))
            {
                /* New connection */
                int newconn = accept(sockfd, NULL, NULL);
                if (newconn < 0)
                {
                    perror("Accept error");
                    return -1;
                }
                
                setnonblocking(newconn);
                /* Assign a slot for the client */
                for (int i = 0; i < 5; i++)
                {
                    if (connections[i] == 0)
                    {
                        printf("\nConnection accepted - FD: %d Slot %d\n", newconn, i);
                        connections[i] = newconn;
                        newconn = -1;
                        break;
                    }
                }
                
                if (newconn != -1)
                {
                    /* Could not find an empty slot */
                    printf("\nConnection refused - no more room\n");
                    
                    sock_puts(newconn, "Server is full, try again later.\r\n");
                    close(newconn);
                }
            }
            
            for(int i = 0; i < 5; i++) 
            {
                if (FD_ISSET(connections[i], &sockets))
                {
                    /* Data received */
                    char buffer [512];
                    if (sock_gets(connections[i], buffer, 512) < 0)
                    {
                        printf("\nConnection lost - FD: %d Slot %d\n", connections[i], i);
                        /* Free up the slot*/
                        close(connections[i]);
                        connections[i] = 0;
                    }
                    else
                    {
                        printf("\nReceived from FD: %d Slot %d:\n\t%s\n", connections[i], i, buffer);
                        /* Echo to all others*/
                        for (int j = 0; j < 5; j++)
                        {
                            if (i == j || connections[j] == 0)
                            {
                                continue;
                            }
                            sock_puts(connections[j], buffer);
                            sock_puts(connections[j], "\n");
                        }
                    }
                }
            }
        }
    }
    /*
    printf("Shutting down");
    
    close(sockfd);
    
    return (EXIT_SUCCESS);
    */
}

