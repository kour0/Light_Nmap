#include "help.h"


command_t help_command = {"help", handle_help, "help [-d]", "Display this help message"};

char *allocate_string(int size) {
    char *str = malloc(size * sizeof(char));
    if (str == NULL) {
        return "Error: Unable to allocate memory for string\n";
    }
    return str;
}


int handle_help(int argc, char *argv[], int client_fd) {

    // Parcourir la liste des commandes
    for (int i = 0; commands[i].command != NULL; i++) {

        char response[256];
        if (argc >= 1 && strcmp(argv[0], "-d") == 0) {
            snprintf(response, sizeof(response), "%s : %s\n", commands[i].command, commands[i].description);
            write(client_fd, response, strlen(response));
        } else {
            snprintf(response, sizeof(response), "%s\n", commands[i].command);
            write(client_fd, response, strlen(response));
        }

        char usage[256];
        snprintf(usage, sizeof(usage), "Usage: %s\n\n", commands[i].usage);
        write(client_fd, usage, strlen(usage));

    }

    return 0;
}

