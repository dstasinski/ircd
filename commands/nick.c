#include "../commands.h"
#include "../util.h"

int command_nick(message_callback_data *callback_data)
{
    info_print_format("Hey from nick, %s %d", callback_data->message_data->command, callback_data->message_data->argc);
    return 0;
}
