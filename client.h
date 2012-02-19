#ifndef CLIENT_H
#define	CLIENT_H

typedef struct client_data
{
    int data;
} client_data;

client_data *client_allocate_new();

#endif	/* CLIENT_H */