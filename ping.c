//
// Created by kour0 on 3/26/24.
//

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define TIMEOUT_SECONDS 5 // Délai d'attente pour la réponse ICMP Echo Reply
#define ICMP_DATA_SIZE 56 // Taille des données à envoyer dans le paquet ICMP

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

// Envoi d'un paquet ICMP Echo Request
void send_echo_request(int sockfd, struct sockaddr_in *dest_addr) {
    static int seq_no = 0; // Numéro de séquence pour les paquets ICMP
    char packet[sizeof(struct icmphdr) + ICMP_DATA_SIZE]; // Paquet ICMP
    struct icmphdr *icmp_hdr = (struct icmphdr *) packet; // En-tête ICMP

    // Configuration de l'en-tête ICMP
    memset(packet, 0, sizeof(packet));
    icmp_hdr->type = ICMP_ECHO;
    icmp_hdr->code = 0;
    icmp_hdr->un.echo.sequence = htons(seq_no++);
    icmp_hdr->un.echo.id = htons(getpid());

    // Ajout de données après l'en-tête ICMP pour simuler une charge utile
    memset(packet + sizeof(struct icmphdr), 'Q', ICMP_DATA_SIZE); // Données arbitraires
    icmp_hdr->checksum = checksum(packet, sizeof(packet));

    // Envoi du paquet
    if (sendto(sockfd, packet, sizeof(packet), 0, (struct sockaddr *) dest_addr, sizeof(*dest_addr)) <= 0) {
        perror("sendto failed");
    }
}

// Réception de la réponse ICMP Echo Reply
void receive_echo_reply(int sockfd) {
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
        perror("select");
    } else if (ret == 0) {
        printf("Ping timed out.\n"); // Délai d'attente dépassé
    } else {
        if (FD_ISSET(sockfd, &readfds)) { // Réception d'un paquet ICMP
            struct sockaddr_in from;
            socklen_t len = sizeof(from);
            if (recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &from, &len) >
                0) { // Lecture du paquet ICMP
                struct iphdr *ip_hdr = (struct iphdr *) buffer;
                struct icmphdr *icmp_hdr = (struct icmphdr *) (buffer + sizeof(struct iphdr));

                // Vérification du type de message ICMP et de l'ID
                if (icmp_hdr->type == ICMP_ECHOREPLY && ntohs(icmp_hdr->un.echo.id) == getpid()) {
                    printf("Ping reply received\n");

                    // On récupère le paquet ICMP Echo Reply
                    printf("Received %d bytes\n", ntohs(ip_hdr->tot_len));
                    printf("TTL: %d\n", ip_hdr->ttl);

                    // On vérifie si le paquet ICMP Echo Reply est valide en vérifiant le checksum
                    unsigned short checksum_received = icmp_hdr->checksum;
                    icmp_hdr->checksum = 0;
                    unsigned short checksum_calculated = checksum(icmp_hdr,
                                                                  ntohs(ip_hdr->tot_len) - sizeof(struct iphdr));
                    icmp_hdr->checksum = checksum_received;
                    if (checksum_received != checksum_calculated) {
                        printf("Invalid checksum\n");
                        return;
                    }

                    // Affichage de l'adresse IP de l'émetteur
                    char sender_ip[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &from.sin_addr, sender_ip, sizeof(sender_ip));
                    printf("Received from: %s\n", sender_ip);

                    // Calcul du temps de réponse
                    struct timeval tv_recv, *tv_send;
                    gettimeofday(&tv_recv, NULL); // Temps de réception
                    tv_send = (struct timeval *) (buffer + sizeof(struct iphdr) +
                                                  sizeof(struct icmphdr)); // Temps d'envoi extrait du paquet

                    // Calcul du RTT
                    long rtt_usec =
                            (tv_recv.tv_sec - tv_send->tv_sec) * 1000000L + (tv_recv.tv_usec - tv_send->tv_usec);
                    printf("RTT: %ld microseconds\n", rtt_usec);
                } else {
                    printf("Received packet was not a ping reply or did not match the sent ID\n");
                }
            } else {
                perror("recvfrom failed");
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <destination IP>\n", argv[0]);
        return 1;
    }

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &dest_addr.sin_addr);

    send_echo_request(sockfd, &dest_addr);
    receive_echo_reply(sockfd);

    close(sockfd);
    return 0;
}
