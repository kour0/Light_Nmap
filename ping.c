#include "ping.h"

command_t ping_command = {"ping", handle_ping, "ping <IP address>", "Send an ICMP echo request to the specified IP address"};

// Calcul du checksum pour l'en-tête ICMP avec les données fournies : b étant l'en-tête ICMP et len sa taille
unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *) buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

int send_echo_request(int sockfd, struct sockaddr_in *dest_addr) {
    char packet[TOTAL_ICMP_SIZE];
    memset(packet, 0, sizeof(packet));

    struct icmp *icmp_hdr = (struct icmp *) packet; // En-tête ICMP
    icmp_hdr->icmp_type = ICMP_ECHO;
    icmp_hdr->icmp_code = 0;
    icmp_hdr->icmp_seq = 0;
    icmp_hdr->icmp_id = htons(getpid());

    // Insérer le timestamp juste après l'en-tête ICMP
    struct timeval *tv_send = (struct timeval *) (packet + ICMP_HEADER_SIZE);
    gettimeofday(tv_send, NULL);

    // Remplir le reste du paquet avec des données
    memset(packet + ICMP_HEADER_SIZE + TIMESTAMP_SIZE, 'A', ICMP_DATA_SIZE);

    icmp_hdr->icmp_cksum = checksum(packet, sizeof(packet));

    // Envoi du paquet
    if (sendto(sockfd, packet, sizeof(packet), 0, (struct sockaddr *) dest_addr, sizeof(*dest_addr)) <= 0) {
        perror("sendto failed");
        return -1;
    }
    return 0;
}

// Réception de la réponse ICMP Echo Reply
int receive_echo_reply(int sockfd, long *rtt_ms, char* ip_response) {
    struct timeval tv;
    fd_set readfds;
    char buffer[1024];
    int ret;

    tv.tv_sec = TIMEOUT_SECONDS;
    tv.tv_usec = 0;

    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);

    // Attente de la réponse ICMP Echo Reply avec un délai de &tv
    ret = select(sockfd + 1, &readfds, NULL, NULL, &tv);

    if (ret == -1) {
        if (errno == EINTR) { // Interruption système
            return 0;
        }
        perror("select");
        return -1;
    } else if (ret == 0) {
        // Timeout
        return 1;
    } else if (FD_ISSET(sockfd, &readfds)) { // Réception d'un paquet ICMP
        // Récupération du paquet ICMP
        struct sockaddr_in from;
        socklen_t len = sizeof(from);
        if (recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &from, &len) <=
            0) {
            perror("recvfrom failed");
            return -1;
        }

        struct timeval tv_recv, *tv_send;
        gettimeofday(&tv_recv, NULL);

        struct ip *ip_hdr = (struct ip *) buffer;
        struct icmp *icmp_hdr = (struct icmp *) (buffer + sizeof(struct ip));

        // Vérification du type de message ICMP et de l'ID
        if (icmp_hdr->icmp_type != ICMP_ECHOREPLY) {
            return -1;
        }

        // On récupère le paquet ICMP Echo Reply
        //printf("Received %d bytes\n", ntohs(ip_hdr->ip_len));
        //printf("TTL: %d\n", ip_hdr->ip_ttl);

        // On vérifie si le paquet ICMP Echo Reply est valide en vérifiant le checksum
        /*unsigned short checksum_received = icmp_hdr->icmp_cksum;
        icmp_hdr->icmp_cksum = 0;
        unsigned short checksum_calculated = checksum(icmp_hdr,
                                                      ntohs(ip_hdr->ip_len) - sizeof(struct ip));
        icmp_hdr->icmp_cksum = checksum_received;
        if (checksum_received != checksum_calculated) {
            printf("Invalid checksum\n");
            return;
        }*/

        // Affichage du timestamp envoyé
        tv_send = (struct timeval *) (buffer + (ip_hdr->ip_hl * 4) + ICMP_HEADER_SIZE);
        
        if (rtt_ms != NULL) {
            *rtt_ms = (tv_recv.tv_sec - tv_send->tv_sec) * 1000 + (tv_recv.tv_usec - tv_send->tv_usec) / 1000;
        }

        if (ip_response != NULL) {
            inet_ntop(AF_INET, &(from.sin_addr), ip_response, INET_ADDRSTRLEN);
        }
    }

    return 0;
}

int simple_ping(struct in_addr ip_addr) {
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr = ip_addr;

    if(send_echo_request(sockfd, &dest_addr) < 0) {
        close(sockfd);
        return -1;
    }

    long rtt_ms;
    if (receive_echo_reply(sockfd, &rtt_ms, NULL) != 0) {
        close(sockfd);
        return -1;
    }
    close(sockfd);
    return 0;
}

int handle_ping(int argc, char *argv[], int client_fd) {

    if (argc != 1) {
        write(client_fd, "Error: Invalid number of arguments\n", 35);
        return -1;
    }

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("socket");
        write(client_fd, "Error: Unable to create socket\n", 31);
        return -1;
    }

    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;

    if (inet_pton(AF_INET, argv[0], &dest_addr.sin_addr) != 1) {
        perror("inet_pton");
        write(client_fd, "Error: Invalid IP address\n", 27);
        close(sockfd);
        return -1;
    }

    if(send_echo_request(sockfd, &dest_addr) < 0) {
        close(sockfd);
        write(client_fd, "Error: Unable to send ping\n", 27);
        return -1;
    }
    write(client_fd, "Ping sent\n", 10);

    long rtt_ms;
    int n;
    n = receive_echo_reply(sockfd, &rtt_ms, NULL);
    if (n < 0) {
        close(sockfd);
        write(client_fd, "Error: Unable to receive ping\n", 30);
        return -1;
    } else if (n == 1) {
        write(client_fd, "Error: Timeout\n", 14);
        return -1;
    }

    char response[100];
    sprintf(response, "Ping response received in RTT = %ld ms\n", rtt_ms);
    write(client_fd, response, strlen(response));

    close(sockfd);
    return 0;
}
