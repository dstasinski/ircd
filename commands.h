#ifndef COMMANDS_H
#define	COMMANDS_H

#include "message.h"

void command_register_handlers();

// Registration
int command_nick(message_callback_data *e);
int command_user(message_callback_data *e);
// Quit
int command_quit(message_callback_data *e);

#endif	/* COMMANDS_H */

