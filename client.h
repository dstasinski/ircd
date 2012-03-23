#ifndef CLIENT_H
#define	CLIENT_H

#include "send.h"

typedef struct client_data
{
    int fd;
    int server;
    
    // RFC: Messages are max 512 bytes in length, including the CR-LF sequence
    // Therefore 510 would be enough for a single, separated message but lets
    // round it up
    // TODO: make it a constant in some global header file
    char line_buffer[512];
    int line_buffer_pos;
    
    // Write queue
    send_queue_element *send_queue_start;
    send_queue_element *send_queue_end; // For fast appending to the queue    
    
    // Linked-list elements
    struct client_data *prev;
    struct client_data *next;
    
    // Nickname hashtable
    char *nickname;
    struct client_data *nickname_next;
    struct client_data *nickname_prev;
    
    // User data
    int registered;
    char *username;
    int quitting;
    
} client_data;

client_data *client_allocate_new();
void client_delete(client_data *client_data);

// TODO: Maybe move somewhere else to remove dependency cycle
typedef struct event_callback_data event_callback_data;
int client_callback_data_in(event_callback_data *e);

client_data *client_nickname_hashtable_find(char *nickname);

void client_set_nickname(client_data *client, const char *nickname);
void client_set_username(client_data *client, const char *username);

#endif	/* CLIENT_H */