#ifndef COMMANDS_H
#define COMMANDS_H

typedef char *(*command_handler_t)(char *args);

typedef struct {
    char *command;
    command_handler_t handler;
} command_t;

char *handle_command1(char *args);
char *handle_command2(char *args);

char *process_command(char *buffer);

#endif // COMMANDS_H

