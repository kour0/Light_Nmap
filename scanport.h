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

int handle_scanport(int argc, char *argv[], int client_fd);

extern command_t scanport_command;

#endif //NMAP_SCANPORT_H
