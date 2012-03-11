#include "../commands.h"
#include "../util.h"

int command_nick(message_callback_data *e)
{
    // TODO: Check valid nick
    // TODO: Check nickname not in use
    
    if (e->message_data->argc < 1)
    {
        // TODO: Reply system for errors
        return 1;
    }
    
    client_set_nickname(e->event_data->client, e->message_data->argv[0]);
    
    return 0;
}

int command_user(message_callback_data *e)
{
    if (e->message_data->argc < 1)
    {
        // TODO: Reply system for errors
        return 1;
    }
    if (e->event_data->client->registered != 0)
    {
        // TODO: ^
        return 2;
    }
    
    client_set_username(e->event_data->client, e->message_data->argv[0]);   
    
    // TODO: Also check for reverse order (USER, then NICK)
    if (e->event_data->client->nickname != NULL)
    {
        e->event_data->client->registered = 1;
        info_print_format("Client registered! nick: %s, username: %s", e->event_data->client->nickname, e->event_data->client->username);
    }
    
    return 0;
}