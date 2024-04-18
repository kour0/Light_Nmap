//
// Created by Alexis on 14/04/2024.
//

#include "scan_port.h"

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
        .usage = "scanport [-f] <IP address> [start port] [end port]\nIf the -f option is present, the scan will be faster (threads).",
        .description = "Scan a port on a remote host, or a range of ports.\n"
};

sem_t sem;

int scan_port(struct sockaddr_in addr) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    int connect_result = connect(sockfd, (struct sockaddr *) &addr, sizeof(addr));
    if (connect_result == -1) {
        close(sockfd);
        if (errno == ECONNREFUSED) {
            // La connexion a été refusée
            return 1;
        } else {
            // Une autre erreur s'est produite
            perror("connect");
            return -1;
        }
    } else {
        close(sockfd);
        return 0;
    }
}

void *scan_port_thread(void *arg) {
    struct thread_data *data = (struct thread_data *)arg;
    if (scan_port(data->addr) == 0) {
        char response[1024];
        sprintf(response, "Port is open: %d\n", ntohs(data->addr.sin_port));
        write(data->client_fd, response, strlen(response));
    }
    free(arg);
    sem_post(&sem); // Increment the semaphore when the thread is done
    return NULL;
}

int handle_scanport_slow(int argc, char *argv[], int client_fd) {
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

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;

    if (inet_pton(AF_INET, ip_address, &addr.sin_addr) != 1) {
        perror("inet_pton");
        write(client_fd, "Error: Invalid IP address\n", 27);
        return -1;
    }

    // write scan port start to end
    char response[1024];
    sprintf(response, "Scanning ports %d to %d\n", start_port, end_port);
    write(client_fd, response, strlen(response));

    for (int port = start_port; port <= end_port; port++) {
        addr.sin_port = htons(port);
        if (scan_port(addr) == 0) {
            sprintf(response, "Port is open: %d\n", port);
            write(client_fd, response, strlen(response));
        }
        printf("%d/%d ports scanned\r", port, end_port);
    }
    return 0;
}

int handle_scanport_fast(int argc, char *argv[], int client_fd) {
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

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;

    if (inet_pton(AF_INET, ip_address, &addr.sin_addr) != 1) {
        perror("inet_pton");
        write(client_fd, "Error: Invalid IP address\n", 27);
        return -1;
    }

    // write scan port start to end
    char response[1024];
    sprintf(response, "Scanning ports %d to %d\n", start_port, end_port);
    write(client_fd, response, strlen(response));

    sem_init(&sem, 0, MAX_THREADS); // Initialize the semaphore

    for (int port = start_port; port <= end_port; port++) {
        
        // Wait for a thread to finish before starting a new one
        sem_wait(&sem);
        
        struct thread_data *data = malloc(sizeof(struct thread_data));
        if (!data) {
            perror("malloc");
            return 1;
        }
        data->addr = addr;
        data->addr.sin_port = htons(port);
        data->client_fd = client_fd;

        pthread_t thread;
        if (pthread_create(&thread, NULL, scan_port_thread, data) != 0) {
            perror("pthread_create");
            free(data);
            return 1;
        }
        // Detach the thread so that its resources are automatically released
        // when it finishes.
        pthread_detach(thread);

        printf("%d/%d ports scanned\r", port, end_port);
    }
    return 0;
}

int handle_scanport(int argc, char *argv[], int client_fd) {
    // On check si le pramètre -f est présent
    if (argc > 0 && strcmp(argv[0], "-f") == 0) {
        return handle_scanport_fast(argc - 1, argv + 1, client_fd);
    } else {
        return handle_scanport_slow(argc, argv, client_fd);
    }
}