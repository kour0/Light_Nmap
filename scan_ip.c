#include "scan_ip.h"
#include "ping.h"
#include <stdio.h>
    #include <sys/wait.h>
#include <signal.h>
#include <sys/socket.h>

volatile sig_atomic_t stop = 0;

command_t scanip_command = {
        .command = "scanip",
        .handler = handle_scanip,
        .usage = "scanip [-f]",
        .description = "Scan all IP addresses on the current network"
};


void handle_sigusr1(int sig) {
    stop = 1;
}

void calculate_network_range(network_t *network, uint32_t *first_ip, uint32_t *last_ip) {
    uint32_t network_address = network->ip & network->netmask;
    uint32_t broadcast_address = network_address | (~network->netmask);
    uint32_t host_bits = 32 - network->size;
    uint32_t host_count = (1 << host_bits) - 2;
    *first_ip = network_address + 1;
    *last_ip = broadcast_address - 1;
}

network_t get_current_network() {
    struct ifaddrs *ifap, *ifa;
    uint32_t loopback_prefix = ntohl(INADDR_LOOPBACK);
    getifaddrs(&ifap);
    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {

        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {

            if (strncmp(ifa->ifa_name, "lo", 2) == 0 || strncmp(ifa->ifa_name, "lo0", 3) == 0) {
                continue;
            }
            struct sockaddr_in *sa = (struct sockaddr_in *) ifa->ifa_addr;
            network_t network = {
                    .ip = ntohl(sa->sin_addr.s_addr),
                    .netmask = ntohl(((struct sockaddr_in *) ifa->ifa_netmask)->sin_addr.s_addr),
                    .size = 0
            };
            uint32_t temp_netmask = network.netmask;
            while (temp_netmask != 0) {
                temp_netmask <<= 1;
                network.size++;
            }
            freeifaddrs(ifap);
            return network;
        }
    }
    freeifaddrs(ifap);
    return (network_t) {0};
}


int handle_scanip_slow(int argc, char *argv[], int client_fd) {

    network_t network = get_current_network();
    uint32_t first_ip, last_ip;
    calculate_network_range(&network, &first_ip, &last_ip);

    char first_ip_str[INET_ADDRSTRLEN];
    char last_ip_str[INET_ADDRSTRLEN];
    struct in_addr first_ip_addr, last_ip_addr;

    first_ip_addr.s_addr = htonl(first_ip);
    inet_ntop(AF_INET, &first_ip_addr, first_ip_str, INET_ADDRSTRLEN);
    last_ip_addr.s_addr = htonl(last_ip);
    inet_ntop(AF_INET, &last_ip_addr, last_ip_str, INET_ADDRSTRLEN);
    printf("Network range: %s - %s\n", first_ip_str, last_ip_str);

    char response[INET_ADDRSTRLEN * 2 + 30];
    snprintf(response, sizeof(response), "Network range: %s - %s\n", first_ip_str, last_ip_str);
    send(client_fd, response, strlen(response), 0);

    for (uint32_t ip = first_ip; ip <= last_ip; ip++) {
        char ip_str[INET_ADDRSTRLEN];
        struct in_addr ip_addr;
        ip_addr.s_addr = htonl(ip);
        inet_ntop(AF_INET, &(ip_addr), ip_str, INET_ADDRSTRLEN);
        printf("Handling IP address: %s\n", ip_str);
        if (simple_ping(ip_addr) == 0) {
            printf("Host is up: %s\n", ip_str);
            char res[INET_ADDRSTRLEN + 30];
            snprintf(res, sizeof(res), "Host is up: %s\n", ip_str);
            send(client_fd, res, strlen(res), 0);
        }
    }

    return 0;
}

int handle_scanip_fast(int argc, char *argv[], int client_fd) {

    network_t network = get_current_network();
    uint32_t first_ip, last_ip;
    calculate_network_range(&network, &first_ip, &last_ip);

    char first_ip_str[INET_ADDRSTRLEN];
    char last_ip_str[INET_ADDRSTRLEN];
    struct in_addr first_ip_addr, last_ip_addr;

    first_ip_addr.s_addr = htonl(first_ip);
    inet_ntop(AF_INET, &first_ip_addr, first_ip_str, INET_ADDRSTRLEN);
    last_ip_addr.s_addr = htonl(last_ip);
    inet_ntop(AF_INET, &last_ip_addr, last_ip_str, INET_ADDRSTRLEN);
    printf("Network range: %s - %s\n", first_ip_str, last_ip_str);

    char response[INET_ADDRSTRLEN * 2 + 30];
    snprintf(response, sizeof(response), "Network range: %s - %s\n", first_ip_str, last_ip_str);
    send(client_fd, response, strlen(response), 0);

    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    // on fait un fork ou le fils écoute les réponses et le père envoie les requêtes
    int pid = fork();
    if (pid == 0) {

        signal(SIGUSR1, handle_sigusr1);

        char* ip_response = malloc(INET_ADDRSTRLEN);
        ip_response[0] = '\0';

        while(!stop) {
            receive_echo_reply(sock, NULL, ip_response);
            if (ip_response[0] != '\0') {
                //printf("Host is up: %s\n", ip_response);
                char res[INET_ADDRSTRLEN + 30];
                snprintf(res, sizeof(res), "Host is up: %s\n", ip_response);
                send(client_fd, res, strlen(res), 0);
                ip_response[0] = '\0';
            }
        }

        free(ip_response);
        close(sock);

        exit(0);

    } else {
        for (uint32_t ip = first_ip; ip <= last_ip; ip++) {
            char ip_str[INET_ADDRSTRLEN];
            struct in_addr ip_addr;
            ip_addr.s_addr = htonl(ip);
            inet_ntop(AF_INET, &(ip_addr), ip_str, INET_ADDRSTRLEN);
            struct sockaddr_in dest_addr;
            memset(&dest_addr, 0, sizeof(dest_addr));
            dest_addr.sin_family = AF_INET;
            dest_addr.sin_addr = ip_addr;
            //printf("Handling IP address: %s\n", ip_str);
            printf("%d/%d IPs scanned\r", ip - first_ip, last_ip - first_ip);
            send_echo_request(sock, &dest_addr);
        }
        printf("\n");
        sleep(TIMEOUT_SECONDS);
        kill(pid, SIGUSR1);
        waitpid(pid, NULL, 0);
    }

    return 0;

}

int handle_scanip(int argc, char *argv[], int client_fd) {

    if (argc >= 1 && strcmp(argv[0], "-f") == 0) {
        return handle_scanip_fast(argc, argv, client_fd);
    } else {
        return handle_scanip_slow(argc, argv, client_fd);
    }
}
