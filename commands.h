#ifndef COMMANDS_H
#define COMMANDS_H


typedef int(*command_handler_t)(int argc, char *argv[], int client_fd);

typedef struct {
    char *command;
    command_handler_t handler;
} command_t;

int process_command(char *buffer, int client_fd);

#endif // COMMANDS_H

