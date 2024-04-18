#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include "commands.h"
#include "ping.h"
#include "scan_ip.h"
#include "help.h"
#include <signal.h>
#include "scan_port.h"

#define PORT 2222
#define BUFFER_SIZE 1024
#define MAX_PENDING 5
int server_fd;

void handle_sigint(int sig)
{
    printf("\nServer is shutting down...\n");
    close(server_fd);
    exit(0);
}

void handle_client(int client_sockfd)
{
    char buffer[BUFFER_SIZE];
    printf("Waiting for command...\n");

    if ((recv(client_sockfd, buffer, BUFFER_SIZE, 0)) <= 0) {
        perror("ERROR reading from socket");
        close(client_sockfd);
        return;
    }
    buffer[strcspn(buffer, "\n")] = 0;

    if (strlen(buffer) == 0) {
        write(client_sockfd, "Error: Empty command\n", 22);
        close(client_sockfd);
        return;
    }

    printf("Command received: %s\n\n", buffer);

    process_command(buffer, client_sockfd);

    memset(buffer, 0, BUFFER_SIZE);
    close(client_sockfd);
}

void init_commands() {
    register_command(&help_command);
    register_command(&ping_command);
    register_command(&scanip_command);
    register_command(&scanport_command);
}

int main(int argc, char *argv[]) {
    printf("Server is starting...\n");

    init_commands();

    struct sockaddr_in serverAddress;

    signal(SIGINT, handle_sigint);

    // Créer une socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    // Vérifier si le port est passé en argument
    if (argc > 1) {
        serverAddress.sin_port = htons(atoi(argv[1]));
    } else {
        serverAddress.sin_port = htons(PORT);
    }

    // Permettre la réutilisation de l'adresse et du port
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

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

    printf("Server is listening on port %d\n\n", ntohs(serverAddress.sin_port));

    while (1) {
        struct sockaddr_in client_addr;
        int client_sockfd;
        socklen_t client_len = sizeof(client_addr);
        printf("Waiting for a connection...\n");
        if ((client_sockfd = accept(server_fd, (struct sockaddr *) &client_addr, &client_len)) <
            0) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }

        printf("Connection accepted from %s\n\n", inet_ntoa(client_addr.sin_addr));

        handle_client(client_sockfd);
    }
    return 0;
}
