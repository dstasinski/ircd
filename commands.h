#ifndef COMMANDS_H
#define	COMMANDS_H

#include "message.h"

void command_register_handlers();

int command_nick(message_callback_data *callback_data);

#endif	/* COMMANDS_H */

