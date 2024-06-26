#ifndef PING_H
#define PING_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netinet/in_systm.h>
#include <netinet/ip_icmp.h>
#include <sys/time.h>
#include <errno.h>
#include "commands.h"

#define TIMEOUT_SECONDS 5
#define ICMP_HEADER_SIZE sizeof(struct icmp)
#define TIMESTAMP_SIZE sizeof(struct timeval)
#define ICMP_DATA_SIZE 56
#define TOTAL_ICMP_SIZE (ICMP_HEADER_SIZE + TIMESTAMP_SIZE + ICMP_DATA_SIZE)

int simple_ping(struct in_addr ip_addr);
int handle_ping(int argc, char *argv[], int client_fd);
int receive_echo_reply(int sockfd, long *rtt_ms, char* ip_response);
int send_echo_request(int sockfd, struct sockaddr_in *dest_addr);

extern command_t ping_command;

#endif //PING_H
