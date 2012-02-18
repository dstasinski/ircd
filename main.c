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
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

/*
 * 
 */
int main(int argc, char** argv) {
    printf("Launching server...\n");
     
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("Socket error: %d\n", errno);
        return -1;
    }
    
    struct sockaddr_in server_addr, client_addr;
    bzero((char *)&server_addr, sizeof(server_addr));
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(6667);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(sockfd, &server_addr, sizeof(server_addr)) < 0)
    {
        printf("Bind error: %d\n", errno);
        return -1;
    }
    
    listen(sockfd, 5);
    printf("Listening for clients\n");
    
    int client_length = sizeof(client_addr);
    int clientfd = accept(sockfd, &client_addr, &client_length);
    if (clientfd < 0)
    {
        printf("Accept error: %d\n", errno);
        return -1;
    }
    
    printf("Got a client\n");
    
    char buffer[256];
    bzero(buffer, 256);
    int n = read(clientfd, buffer, 255);
    if (n < 0)
    {
        printf("Read error: %d\n", errno);
        return -1;
    }
    
    printf("Message from client: %s\n", buffer);
    
    n = write(clientfd, "Hello!", 6);
    if (n < 0)
    {
        printf("Write error: %d\n", errno);
        return -1;
    }
    
    printf("Shutting down");
    
    close(clientfd);
    close(sockfd);
    
    return (EXIT_SUCCESS);
}

