#include "client.h"

client_data *client_allocate_new()
{
    client_data *client = (client_data *) malloc(sizeof(client));
    client->data = 0;
    
    return client;
}
