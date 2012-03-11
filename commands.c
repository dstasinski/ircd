#include "commands.h"
#include "message.h"

void command_register_handlers()
{
    message_register_handler("nick", command_nick);
}
