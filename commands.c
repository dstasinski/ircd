#include "commands.h"
#include "message.h"

void command_register_handlers()
{
    message_register_handler("nick", command_nick);
    message_register_handler("user", command_user);
    
    message_register_handler("quit", command_quit);
}
