//
// Created by Alexis on 05/03/2024.
//

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {

    struct sockaddr_in serv_addr;
    int sockfd;

    char buffer[BUFFER_SIZE] = {0};

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons((ushort) atoi(argv[2]));
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        exit(1);
    }
    printf("Connected to server\n");
    printf("See available commands with 'help'\n");
    printf("Enter a command: ");
    fgets(buffer, 255, stdin);
    ssize_t n = send(sockfd, buffer, strlen(buffer), 0);
    if (n < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }

    memset(buffer, 0, BUFFER_SIZE);

    printf("Server response:\n\n");
    while ((n = recv(sockfd, buffer, BUFFER_SIZE, 0)) > 0) {
        printf("%s", buffer);
        memset(buffer, 0, BUFFER_SIZE);
    }
    if (n < 0) {
        perror("ERROR reading from socket");
        exit(1);
    }

}

