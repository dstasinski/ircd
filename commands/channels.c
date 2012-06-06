#include "../commands.h"
#include "../channel.h"
#include "../send.h"
#include "../util.h"

int command_join(message_callback_data *e)
{
    if (e->event_data->client->registered != 1)
    {
        command_user_reply(e, "451 :You have not registered");
        return -1;
    }
    
    if (e->message_data->argc < 1)
    {
        command_user_reply(e, "461 JOIN :Not enough parameters");
        return -1;
    }
    if (e->message_data->argc > 1)
    {
        command_user_reply(e, "405 :Too many channels");
        return -1;
    }
    
    
    channel_data *channel = channel_hashtable_find(e->message_data->argv[0]);
    if (channel == NULL)
    {
        /* Create new channel */
        info_print_format("creating new channel %s", e->message_data->argv[0]);
        channel = channel_create_new(e->message_data->argv[0]);
    }
    
    /* Avoid duplicates */
    channel_client *c_client = channel->clients;
    while(c_client != NULL)
    {
        if (c_client->client == e->event_data->client)
        {
            /* Already in this channel, ignore */
            return -1;
        }
        
        c_client = c_client->next;
    }
    
    // Broadcast the join event to other channel members
    send_message_buffer *buffer = send_create_buffer_format(":%s JOIN %s", e->event_data->client->nickname, e->message_data->argv[0]);
    send_enqueue_channel(channel, buffer);
    
    channel_client_join(channel, e->event_data->client);
    
    return 0;
}

int command_part(message_callback_data *e)
{
    if (e->event_data->client->registered != 1)
    {
        command_user_reply(e, "451 :You have not registered");
        return -1;
    }
    
    if (e->message_data->argc < 1)
    {
        command_user_reply(e, "461 PART :Not enough parameters");
        return -1;
    }
    if (e->message_data->argc > 1)
    {
        command_user_reply(e, "405 :Too many channels");
        return -1;
    }
    
    channel_data *channel = channel_hashtable_find(e->message_data->argv[0]);
    if (channel == NULL)
    {
        command_user_reply_format(e, "403 %s :No such channel", e->message_data->argv[0]);
        return -1;
    }
    
    /* Remove if on channel, otherwise return error */
    int res = channel_client_part(channel, e->event_data->client);
    if (res == 0)
    {
        command_user_reply_format(e, "442 %s :Not on channel", e->message_data->argv[0]);
        return -1;
    }
    
    /* If the channel wasn't deleted */
    if (res != -1)
    {
        send_message_buffer *buffer = send_create_buffer_format(":%s PART %s", e->event_data->client->nickname, e->message_data->argv[0]);
        send_enqueue_channel(channel, buffer);
    }
    
    return 0;
}