#include "scan_ip.h"
#include "ping.h"

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
            printf("Interface: %s\n", ifa->ifa_name);
            printf("IP address: %s\n", inet_ntoa(sa->sin_addr));
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

int handle_scanip(int argc, char *argv[], int client_fd) {
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

    for (uint32_t ip = first_ip; ip <= last_ip; ip++) {
        char ip_str[INET_ADDRSTRLEN];
        struct in_addr ip_addr;
        ip_addr.s_addr = htonl(ip);
        inet_ntop(AF_INET, &(ip_addr), ip_str, INET_ADDRSTRLEN);
        printf("Handling IP address: %s\n", ip_str);
        if (simple_ping(ip_addr) == 0) {
            printf("Host is up: %s\n", ip_str);
        }
    }
    return 0;
}
