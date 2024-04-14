#include "help.h"


command_t help_command = {"help", handle_help, "Display this help message"};

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
        write(client_fd, "\n", 1);

        // Envoyer les paramètres de la commande au client
        if (commands[i].usage != NULL) {
            // Allouer de la mémoire pour la chaîne de format
            char *format = allocate_string(strlen(commands[i].usage) + argc * 2 + 1);
            if (format == NULL) {
                write(client_fd, "Error: Unable to allocate memory for format string\n", 51);
                return -1;
            }

            // Copier la chaîne d'utilisation dans la chaîne de format
            strcpy(format, commands[i].usage);

            // Remplacer les paramètres optionnels par leurs valeurs par défaut
            for (int j = 0; j < argc; j++) {
                char param[32];
                snprintf(param, sizeof(param), "[-%c %s]", argv[j][0], argv[j+1]);
                char *default_value = "";
                if (strcmp(param, "-c") == 0) {
                    default_value = "5";
                } else if (strcmp(param, "-i") == 0) {
                    default_value = "1";
                } else if (strcmp(param, "-s") == 0) {
                    default_value = "56";
                }
                char *option = strstr(format, param);
                if (option != NULL) {
                    char replacement[32];
                    snprintf(replacement, sizeof(replacement), "[-%c %s]", argv[j][0], default_value);
                    strncpy(option, replacement, strlen(replacement));
                }
            }

            // Envoyer la chaîne de format au client
            write(client_fd, format, strlen(format));
            write(client_fd, "\n", 1);

            // Libérer la mémoire allouée pour la chaîne de format
            free(format);
        }
    }

    return 0;
}

