#include <stdlib.h>

#include "client.h"

client_data *clients = NULL;

client_data *client_allocate_new()
{
    client_data *client = malloc(sizeof(client_data));
    client->data = 0;
    client->name = NULL;
    client->prev = NULL;
    client->next = clients;
    
    clients = client;
    
    return client;
}

void client_delete(client_data* client_data)
{
    if (client_data->prev != NULL)
    {
        client_data->prev->next = client_data->next;
    }
    else
    {
        clients = client_data->next;
    }
    
    if (client_data->name != NULL)
    {
        free(client_data->name);
    }
    free(client_data);
}