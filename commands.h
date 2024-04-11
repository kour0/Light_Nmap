#ifndef COMMANDS_H
#define COMMANDS_H

typedef char *(*command_handler_t)(int argc, char *argv[]);

typedef struct {
    char *command;
    command_handler_t handler;
} command_t;

char *handle_command1(int argc, char *argv[]);
char *handle_command2(int argc, char *argv[]);

char *process_command(char *buffer);

#endif // COMMANDS_H

