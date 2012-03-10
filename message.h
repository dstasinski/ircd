#ifndef MESSAGE_H
#define	MESSAGE_H

#include "rfc.h"

typedef struct message_data
{
    // TODO: parse prefix, verify, maybe use dynamic allocation
    char prefix[RFC_PREFIX_MAXLENGTH];
    
    char command[RFC_COMMAND_MAXLENGTH];
    int command_numeric;
    
    int argc;
    char *argv[RFC_MAX_ARGUMENTS];
} message_data;

message_data *message_parse(char *buffer);
void message_delete(message_data *message);

#endif	/* MESSAGE_H */

