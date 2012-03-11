#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "client.h"
#include "event.h"
#include "message.h"

client_data *clients = NULL;

client_data *client_allocate_new()
{
    client_data *client = malloc(sizeof(client_data));
    client->fd = 0;
    client->server = 0;
    
    client->line_buffer_pos = 0;
    
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

int client_callback_data(event_callback_data *e)
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
            printf("message |%s| parsed as\n", e->client->line_buffer);
            if (message != NULL)
            {
                printf("\t |%s| (%d), argc=%d\n", message->command, message->command_numeric, message->argc);
                for(int j = 0; j < message->argc; j++)
                {
                    printf("\t %d |%s|\n", j, message->argv[j]);
                }
                
                // TODO: Make static / reuse inside the function / something else
                message_callback_data *callback_data = malloc(sizeof(message_callback_data));
                callback_data->event_data = e;
                callback_data->message_data = message;
                message_dispatch_command(message->command, callback_data);
                free(callback_data);
            }
            else
            {
                printf("\t NULL\n");
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