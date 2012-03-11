#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

int _error_print(const char *scope, const char *call, const char *file, int line)
{
    fprintf(stderr, "Error: [%s] %s\n\tIn file %s at line %d\n\t", scope, call, file, line);
    perror(NULL);
    return -1;
}

int _error_print_exit(const char *scope, const char *call, const char *file, int line)
{
    _error_print(scope, call, file, line);
    exit(EXIT_FAILURE);
}

void _info_print(const char *message)
{
    printf("Info: %s\n", message);
}

void _info_print_format(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    
    printf("\n");
}

unsigned long hash(const char *string)
{
    // djb2 - simple string hashing
    
    unsigned long hash = 5381;
    int c;
    
    while (c = *string++)
    {
        hash = ((hash << 5) + hash) + c; // hash*33 + c
    }
    
    return hash;
}