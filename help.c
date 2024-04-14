#include "help.h"


command_t help_command = {"help", handle_help, "help", "Display this help message"};

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
        // Envoyer le nom de la commande au client
        write(client_fd, commands[i].command, strlen(commands[i].command));
        write(client_fd, ": ", 2);

        write(client_fd, commands[i].description, strlen(commands[i].description));
        write(client_fd, "\n", 1);

        write(client_fd, "Usage: ", 7);
        write(client_fd, commands[i].usage, strlen(commands[i].usage));
        write(client_fd, "\n", 1);
    }

    return 0;
}

