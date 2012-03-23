#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "client.h"
#include "event.h"
#include "message.h"
#include "util.h"
#include "send.h"
#include "common.h"

void client_nickname_hashtable_add(client_data *client);
void client_nickname_hashtable_remove(client_data *client);

static client_data *clients = NULL;
static client_data *client_nickname[CLIENT_NICKNAME_HASHTABLE_SIZE];

client_data *client_allocate_new()
{
    client_data *client = malloc(sizeof(client_data));
    client->fd = 0;
    client->server = 0;
    
    client->line_buffer_pos = 0;
    
    client->send_queue_start = NULL;
    client->send_queue_end = NULL;
    
    client->prev = NULL;
    client->next = clients;
    
    client->nickname = NULL;
    client->nickname_next = NULL;
    client->nickname_prev = NULL;
    
    client->registered = 0;
    client->username = NULL;
    client->quitting = 0;
    
    // Prepend to the client list
    clients = client;
    return client;
}

void client_delete(client_data* client)
{
    // Remove from the linked list of clients
    if (client->prev != NULL)
    {
        client->prev->next = client->next;
    }
    else
    {
        clients = client->next;
    }
    
    // Clean up all data stored in this client's client_data
    if (client->send_queue_start != NULL)
    {
        send_delete_queue(client->send_queue_start);
    }
    
    // Remove from nickname hashtable
    client_nickname_hashtable_remove(client);
    
    // TODO: static allocation, or macros or something to avoid this mess
    if (client->nickname != NULL)
    {
        free(client->nickname);
    }
    if (client->username != NULL)
    {
        free(client->username);
    }
    
    free(client);
}

int client_callback_data_in(event_callback_data *e)
{
    // TODO: Doesn't work when CR is at the end of one buffer and LF at the
    // beginning of the next one.
    
    const char *start = e->buffer;
    const char *pos = start+1;
    for(int i = 1; i < e->buffer_length; i++)
    {
        if (*(pos-1) == '\r' && *pos == '\n')
        {
            // Found line end, append everything from start to pos-1 to the
            // line buffer and process it
            int length = pos - start - 1;
            if (e->client->line_buffer_pos + length >= sizeof(e->client->line_buffer))
            {
                length = sizeof(e->client->line_buffer) - e->client->line_buffer_pos - 1;
            }
            memcpy(e->client->line_buffer + e->client->line_buffer_pos, start, length);
            e->client->line_buffer_pos += length;
            e->client->line_buffer[e->client->line_buffer_pos] = '\0';
            
            // Parse the line into a message structure
            message_data *message = message_parse(e->client->line_buffer);
            
            // Process the message
            info_print_format("Got message %s", e->client->line_buffer); // TODO: Change to debug print, not info
            if (message != NULL)
            {
                info_print_format("Command [%s] argc=%d", message->command, message->argc);
                
                // TODO: Make static / reuse inside the function / something else
                message_callback_data *callback_data = malloc(sizeof(message_callback_data));
                callback_data->event_data = e;
                callback_data->message_data = message;
                message_dispatch_command(message->command, callback_data);
                free(callback_data);
            }
            
            // Clean up
            message_delete(message);
            
            e->client->line_buffer_pos = 0;
            start = pos+1;
        }
        pos++;
    }
    // Reached the end, copy any remaining unterminated characters to the buffer
    if (start != pos)    
    {
        memcpy(e->client->line_buffer, start, pos - start);
        e->client->line_buffer_pos = pos - start;
    }
    
    return 0;
}

// TODO: Lowercase (RFC lowercase!) hashing and comparison

void client_nickname_hashtable_add(client_data *client)
{
    int bucket = hash(client->nickname) % CLIENT_NICKNAME_HASHTABLE_SIZE;
    
    if (client_nickname[bucket] == NULL)
    {
        client_nickname[bucket] = client;
    }
    else
    {
        client->nickname_next = client_nickname[bucket];
        client_nickname[bucket] = client;
    }
}

void client_nickname_hashtable_remove(client_data *client)
{
    if (client->nickname_prev == NULL)
    {
        client_nickname[hash(client->nickname) % CLIENT_NICKNAME_HASHTABLE_SIZE] = client->nickname_next;
    }
    else
    {
        client->nickname_prev->nickname_next = client->nickname_next;
    }
}

client_data *client_nickname_hashtable_find(char *nickname)
{
    client_data *client = client_nickname[hash(nickname) % CLIENT_NICKNAME_HASHTABLE_SIZE];
    while (client != NULL)
    {
        if (strcmp(client->nickname, nickname) == 0)
        {
            return client;
        }
        client = client->nickname_next;
    }
    
    return NULL;
}

void client_set_nickname(client_data *client, const char *nickname)
{
    if (client->nickname != NULL)
    {
        client_nickname_hashtable_remove(client);
        free(client->nickname);
    }
    
    client->nickname = malloc(sizeof(char)*(strlen(nickname)+1));
    strcpy(client->nickname, nickname);
    
    client_nickname_hashtable_add(client);    
}

void client_set_username(client_data *client, const char *username)
{
    if (client->username != NULL)
    {
        free(client->username);
    }
    client->username = malloc(sizeof(char)*(strlen(username)+1));
    strcpy(client->username, username);
}