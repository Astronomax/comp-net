#ifdef WIN32
#include <winsock2.h>
#include <wspiapi.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <netdb.h>
#endif

#include <iostream>
#include <string>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <cstdlib>

#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <cassert>
#include <chrono>

#ifdef WIN32
struct icmphdr
{
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t id;
    uint16_t sequence;
};
#endif

uint16_t checksum(const void *data, size_t len) {
    auto p = reinterpret_cast<const uint16_t*>(data);
    uint32_t sum = 0;
    if (len % 2 == 1) {
        assert(false);
    }
    for(int i = 0; i < len / 2; i++) {
        sum += *(p + i);
    }
    return static_cast<uint16_t>(~sum);
}

void sock_close(int sock_fd) {
#ifdef WIN32
    closesocket(sock_fd);
#else
    close(sock_fd);
#endif
}

int main(int argc, char *argv[]) {
    if (argc < 1) {
        fprintf(stderr, "Too few arguments. Expected {url}`\n");
        return 1;
    }

#ifdef WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    std::string hostname(argv[1]);
    const size_t payload_size = 64;
    int sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    if (sock_fd < 0) {
        fprintf(stderr, "Error occurred while opening socket\n");
        return 1;
    }

    hostent *host = gethostbyname(hostname.data());
    if (host == nullptr) {
        fprintf(stderr, "Error: host not found\n");
        exit(1);
    }

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    memcpy((char*)&serv_addr.sin_addr.s_addr,(char*)host->h_addr, host->h_length);

    struct icmphdr icmp_hdr{};
    memset(&icmp_hdr, 0, sizeof icmp_hdr);
    icmp_hdr.type = 8;
    #ifdef WIN32
        icmp_hdr.id = 1024;
    #else
        icmp_hdr.un.echo.id = 1024;
    #endif
    uint16_t sequence = 1;
    char data[2048];
    const int MAX_HOPS_NUMBER = 30;
    printf("\nTracing route to %s [%s]\nover a maximum of %d hops:\n\n", hostname.c_str(), inet_ntoa(serv_addr.sin_addr), MAX_HOPS_NUMBER);

    for (int ttl = 1; ttl < MAX_HOPS_NUMBER; ttl++) {
        printf("%2d", ttl);
        setsockopt(sock_fd, IPPROTO_IPV6, IP_TTL, (char*)&ttl, sizeof(ttl));
        setsockopt(sock_fd, IPPROTO_IP, IP_TTL, (char*)&ttl, sizeof(ttl));

        int8_t type;
        struct sockaddr_in src_addr{};
        bool response_received = false;
        for(int j = 0; j < 3; j++) {
            memset(data, 0, sizeof(icmp_hdr) + payload_size);
            #ifdef WIN32
                icmp_hdr.sequence = sequence++;
            #else
                icmp_hdr.un.echo.sequence = sequence++;
            #endif
            icmp_hdr.checksum = 0;
            memcpy(data, &icmp_hdr, sizeof icmp_hdr); //add header
            icmp_hdr.checksum = checksum(reinterpret_cast<unsigned short*>(data), sizeof(icmp_hdr) + payload_size);
            memcpy(data, &icmp_hdr, sizeof icmp_hdr);
            //setsockopt(sock_fd, IPPROTO_IPV6, IP_TTL, (char*)&ttl, sizeof(ttl));
            auto start = std::chrono::system_clock::now();
            ssize_t rc = sendto(sock_fd, data, sizeof(icmp_hdr) + payload_size, 0, (struct sockaddr *) &serv_addr, sizeof serv_addr);
            if (rc <= 0) {
                fprintf(stderr, "Error occurred while sending data\n");
                return 1;
            }
            fd_set read_set;
            struct timeval timeout = {2, 0};
            memset(&read_set, 0, sizeof read_set);
            FD_SET(sock_fd, &read_set);
            rc = select(sock_fd + 1, &read_set, nullptr, nullptr, &timeout);
            if (rc == 0) {
                printf("     *   ");
                continue;
            } else if (rc < 0) {
                fprintf(stderr, "Error occurred while receiving data\n");
                return 1;
            }
            rc = recv(sock_fd, data, sizeof data, 0);
            auto end = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_seconds = end - start;
            if (rc <= 0) {
                fprintf(stderr, "Error occurred while receiving data\n");
                return 1;
            } else if (rc < sizeof icmp_hdr) {
                fprintf(stderr, "Error, got short ICMP packet, %zd bytes\n", rc);
                return 1;
            }

            memcpy(&src_addr.sin_addr.s_addr, data + 12, sizeof src_addr.sin_addr.s_addr);
            printf("%6.0f ms", elapsed_seconds.count() * 1000);
            type = *reinterpret_cast<int8_t *>(data + 20);
            response_received = true;
        }
        if(response_received) {
            printf(" [%s]\n", inet_ntoa(src_addr.sin_addr));
            if (type == 0) {
                break;
            }
        } else {
            printf("  The waiting interval for the request has been exceeded.\n");
        }
    }
    printf("\nTrace complete.\n");
    sock_close(sock_fd);
    return 0;
}
