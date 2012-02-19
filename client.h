#ifndef CLIENT_H
#define	CLIENT_H

typedef struct client_data
{
    int data;
    char *name;
    
    struct client_data *prev;
    struct client_data *next;
} client_data;

extern client_data *clients;

client_data *client_allocate_new();
void client_delete(client_data *client_data);

#endif	/* CLIENT_H */