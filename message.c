#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "message.h"


message_data *message_parse(char *buffer)
{
    message_data *message = malloc(sizeof(message_data));
    // TODO: More checks for \0, return null for malformed messages
    
    char *start;
    int length;
    
    // Check for prefix
    if (*buffer == ':')
    {
        // Move to the first character of the prefix
        start = ++buffer;
        // Find fist space, everything from start to there is the prefix
        buffer = strchr(buffer, ' ');
        if (buffer == NULL)
        {
            free(message);
            return NULL;
        }
        
        length = buffer - start;
        // TODO: Check that length < sizeof(->prefix)
        strncpy(message->prefix, start, length);
        message->prefix[length] = '\0';
        
        buffer++; // Move behind the space
    }
    
    // Parse the command
    start = buffer;
    buffer = strchr(buffer, ' ');
    if (buffer == NULL)
    {
        buffer = strchr(start, '\0');
    }
    length = buffer - start;
    strncpy(message->command, start, length);
    message->command[length] = '\0';
    
    message->command_numeric = 0;
    if (length == 3)
    {
        // Might be numeric message
        message->command_numeric = atoi(message->command);
    }
    
    // Move behind the space, if there are any arguments
    if (*buffer != '\0')
    {
        buffer++;
    }
    
    // Parse arguments
    message->argc = 0;
    while(*buffer != '\0')
    {
        start = buffer;
        if (*start == ':')  
        {
            // Trailing argument
            buffer = strchr(buffer, '\0');
            start++;
        }
        else
        {
            buffer = strchr(buffer, ' ');
            if (buffer == NULL)
            {
                // Last argument, find end of string instead
                buffer = strchr(start, '\0');
            }
        }
        
        length = buffer - start;
        message->argv[message->argc] = malloc(sizeof(char)*(length+1));
        strncpy(message->argv[message->argc], start, length);
        message->argc++;     
        
        // Move behind the space, if any
        if (*buffer != '\0')
        {
            buffer++;
        }
    }
    
    return message;
}

void message_delete(message_data *message)
{
    if (message == NULL)
    {
        return;
    }
    
    for(int i = 0; i < message->argc; i++)
    {
        free(message->argv[i]);
    }
    free(message);
}