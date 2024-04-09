#ifndef PING_H
#define PING_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netinet/in_systm.h>
#include <netinet/ip_icmp.h>
#include <sys/time.h>

#define TIMEOUT_SECONDS 5
#define ICMP_HEADER_SIZE sizeof(struct icmp)
#define TIMESTAMP_SIZE sizeof(struct timeval)
#define ICMP_DATA_SIZE 56
#define TOTAL_ICMP_SIZE (ICMP_HEADER_SIZE + TIMESTAMP_SIZE + ICMP_DATA_SIZE)

unsigned short checksum(void *b, int len);
void send_echo_request(int sockfd, struct sockaddr_in *dest_addr);
void receive_echo_reply(int sockfd);

#endif //PING_H
