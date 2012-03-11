#include <unistd.h>

#include "../commands.h"
#include "../util.h"

int command_quit(message_callback_data *e)
{
    // TODO: Remove from channels, announce quit, etc
    // Call some helper function to do that, reason in parameters
    // Also call that function when there is a socket error, ping timeout etc
    
    e->event_data->client->quitting = 1;
    close(e->event_data->client->fd);
    return 0;
}
