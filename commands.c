#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "commands.h"

char *handle_command1(char *args) {
    // Traiter les arguments et exécuter l'action personnalisée pour la commande 1
    char *response = malloc(100 * sizeof(char));
    snprintf(response, 100, "Command 1 processed with arguments: %s\n", args);
    return response;
}

char *handle_command2(char *args) {
    // Traiter les arguments et exécuter l'action personnalisée pour la commande 2
    char *response = malloc(100 * sizeof(char));
    snprintf(response, 100, "Command 2 processed with arguments: %s\n", args);
    return response;
}

command_t commands[] = {
        {"command1", handle_command1},
        {"command2", handle_command2},
        {NULL, NULL} // Marqueur de fin de tableau
};

char *process_command(char *buffer) {
    char *token, *args;
    char *response = NULL;

    // Tokeniser la chaîne reçue pour extraire la commande et les arguments
    token = strtok(buffer, " \n");
    args = token ? strtok(NULL, " \n") : NULL;

    // Parcourir le tableau des commandes et appeler le gestionnaire correspondant
    int i = 0;
    while (commands[i].command != NULL) {
        if (strcmp(token, commands[i].command) == 0) {
            response = commands[i].handler(args);
            break;
        }
        i++;
    }

    // Si la commande n'est pas trouvée, renvoyer un message d'erreur
    if (commands[i].command == NULL) {
        response = malloc(50 * sizeof(char));
        snprintf(response, 50, "Unknown command: %s\n", token);
    }

    return response;
}
