//
// Created by Alexis on 14/04/2024.
//

#include "scanport.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

command_t scanport_command = {
        .command = "scanport",
        .handler = handle_scanport,
        .usage = "scanport <IP address> [start port] [end port]",
        .description = "Scan a port on a remote host"
};


int scan_port(struct sockaddr_in addr) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    int connect_result = connect(sockfd, (struct sockaddr *) &addr, sizeof(addr));
    if (connect_result == -1) {
        close(sockfd);
        return 1;
    } else {
        close(sockfd);
        return 0;
    }
}

int handle_scanport(int argc, char *argv[], int client_fd) {
    int start_port = 1;
    int end_port = 65535;
    if (argc < 1) {
        printf("Usage: scanport <IP address> <start port> <end port>\n");
        write(client_fd, "Usage: scanport <IP address> <start port> <end port>\n", 51);
        return 1;
    }
    const char *ip_address = argv[0];
    if (argc == 3) {
        start_port = atoi(argv[1]);
        end_port = atoi(argv[2]);
    }

    // write scan port start to end
    char response[1024];
    sprintf(response, "Scanning ports %d to %d\n", start_port, end_port);
    write(client_fd, response, strlen(response));


    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip_address, &addr.sin_addr);

    for (int port = start_port; port <= end_port; port++) {
        addr.sin_port = htons(port);
        if (scan_port(addr) == 0) {
            printf("Port is open: %d\n", port);
            char response[1024];
            sprintf(response, "Port is open: %d", port);
            write(client_fd, response, strlen(response));
        }
        printf("%d/%d ports scanned\r", port, end_port);
    }
    return 0;
}
