#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include "commands.h"

#define PORT 2222
#define BUFFER_SIZE 1024
#define MAX_PENDING 5

int main() {
    printf("Server is starting...\n");
    int server_fd, client_fd;
    struct sockaddr_in serverAddress;
    struct sockaddr_in clientAddress;
    int clientAddressLength;


    char buffer[BUFFER_SIZE] = {0};
    char* response;

    // Créer une socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    // Lier la socket à l'adresse et au port
    if (bind(server_fd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Écouter les connexions entrantes
    if (listen(server_fd, MAX_PENDING) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", PORT);

    if ((client_fd = accept(server_fd, (struct sockaddr *) &clientAddress, (socklen_t *) &clientAddressLength)) <
        0) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }

    while (1) {

        ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
        if (bytes_received < 0) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        printf("Received command: %s\n", buffer);

        memset(buffer, 0, BUFFER_SIZE);

        response = process_command(buffer);

        printf("Response: %s\n", response);

        // Envoyer la réponse au client
        send(client_fd, response, strlen(response), 0);

        free(response);

    }

    close(client_fd);
    return 0;
}
