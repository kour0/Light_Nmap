//
// Created by Alexis on 14/04/2024.
//

#ifndef NMAP_SCANPORT_H
#define NMAP_SCANPORT_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "commands.h"
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <netinet/in.h>

#define MAX_THREADS 1000

struct thread_data {
    struct sockaddr_in addr;
    int client_fd;};

int handle_scanport(int argc, char *argv[], int client_fd);

extern command_t scanport_command;

#endif //NMAP_SCANPORT_H
