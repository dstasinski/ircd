#include <string.h>

#include "../commands.h"
#include "../client.h"

int command_privmsg(message_callback_data* e)
{
    if (e->message_data->argc < 1)
    {
        command_user_reply(e, "411 :No recipient");
        return -1;
    }
    if (e->message_data->argc != 2)
    {
        command_user_reply(e, "412 :No text to send");
        return -1;
    }
    
    send_message_buffer *buffer = send_create_buffer_format(":%s PRIVMSG %s :%s", e->event_data->client->nickname, e->message_data->argv[0], e->message_data->argv[1]);
    
    if (e->message_data->argv[0][0] == '#')
    {
        /* Channel */
    }
    else 
    {
        /* User */
        client_data *target = client_nickname_hashtable_find(e->message_data->argv[0]);
        if (target == NULL)
        {
            command_user_reply(e, "401 :No such nick");
            return -1;
        }
        
        send_enqueue_client(target, buffer);
    }
    
    return 0;
}