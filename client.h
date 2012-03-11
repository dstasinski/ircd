#ifndef CLIENT_H
#define	CLIENT_H

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
    
    struct client_data *prev;
    struct client_data *next;
    
    // User data
    int registered;
    char *nickname;
    char *username;
    int quitting;
    
} client_data;

extern client_data *clients;

client_data *client_allocate_new();
void client_delete(client_data *client_data);

// TODO: Maybe move somewhere else to remove dependency cycle
typedef struct event_callback_data event_callback_data;
int client_callback_data(event_callback_data *e);

void client_set_nickname(client_data *client, const char *nickname);
void client_set_username(client_data *client, const char *username);

#endif	/* CLIENT_H */