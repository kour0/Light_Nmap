#ifndef NMAP_INFO_H
#define NMAP_INFO_H

#include <stdio.h>
#include <string.h>
#include "commands.h"

extern command_t commands[];

int handle_help(int argc, char *argv[], int client_fd);

extern command_t help_command;

#endif //NMAP_INFO_H
