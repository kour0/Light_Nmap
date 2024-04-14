#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

typedef int(*command_handler_t)(int argc, char *argv[], int client_fd);

typedef struct {
    char *command;
    command_handler_t handler;
    const char *usage;
    const char *description;
} command_t;

extern command_t commands[];

int process_command(char *buffer, int client_fd);

void register_command(command_t *command);

#endif // COMMANDS_H

