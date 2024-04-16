//
// Created by Alexis on 12/04/2024.
//

#ifndef NMAP_SCAN_IP_H
#define NMAP_SCAN_IP_H

#include <stdio.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <string.h>
#include "commands.h"

typedef struct {
    uint32_t ip;
    uint32_t netmask;
    uint32_t size;
} network_t;

int handle_scanip(int argc, char *argv[], int client_fd);

extern command_t scanip_command;

#endif //NMAP_SCAN_IP_H

