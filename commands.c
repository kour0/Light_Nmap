#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "commands.h"
#include "ping.h"

 int handle_command1(int argc, char *argv[], int client_fd) {
    send(client_fd, "Command 1\n", 10, 0);
    return 0;
}

int handle_command2(int argc, char *argv[], int client_fd) {
    send(client_fd, "Command 2\n", 10, 0);
    return 0;
}


command_t commands[] = {
        {"command1", handle_command1},
        {"command2", handle_command2},
        {"ping", handle_ping},
        {NULL, NULL} // Marqueur de fin de tableau
};

void free_args(char **args, int arg_count) {
    for (int i = 0; i < arg_count; i++) {
        free(args[i]);
    }
    free(args);
}

char *allocate_string(int size) {
    char *str = malloc(size * sizeof(char));
    if (str == NULL) {
        return "Error: Unable to allocate memory for string\n";
    }
    return str;
}

int split_string(char *buffer, char ***args)
{
    int arg_count = 0;
    char *token;
    char *delimiter = " \t\n";
    char *buffer_copy = strdup(buffer); // Duplicate the buffer

    if (buffer_copy == NULL) {
        return -1;
    }

    token = strtok(buffer_copy, delimiter);
    while (token != NULL)
    {
        arg_count++;
        token = strtok(NULL, delimiter);
    }

    free(buffer_copy); // Free the duplicated buffer
    buffer_copy = strdup(buffer); // Duplicate the buffer again for the second pass

    if (buffer_copy == NULL) {
        return -1;
    }

    *args = (char **)malloc((arg_count + 1) * sizeof(char *));
    if (*args == NULL)
    {
        free(buffer_copy);
        return -1;
    }

    arg_count = 0;
    token = strtok(buffer_copy, delimiter);
    while (token != NULL)
    {
        (*args)[arg_count] = strdup(token); // Duplicate the token
        arg_count++;
        token = strtok(NULL, delimiter);
    }
    (*args)[arg_count] = NULL; // Ajouter un pointeur nul à la fin

    free(buffer_copy); // Free the duplicated buffer

    return arg_count;
}

command_handler_t find_command_handler(const char *command) {
    int i = 0;
    while (commands[i].command != NULL) {
        if (strcmp(command, commands[i].command) == 0) {
            return commands[i].handler;
        }
        i++;
    }
    return NULL;
}

int process_command(char *buffer, int client_fd) {
    char **args;
    int arg_count = split_string(buffer, &args);

    if (arg_count < 0) {
        write(client_fd, "Error: invalid command\n", 23);
        return -1;
    }

    command_handler_t handler = find_command_handler(args[0]);

    if (handler == NULL) {
        free_args(args, arg_count);
        printf("The command %s was not found\n", args[0]);
        write(client_fd, "Command not found\n", 18);
        return -1;
    }

    handler(arg_count - 1, args + 1, client_fd);
    free_args(args, arg_count);

    return 0;
}