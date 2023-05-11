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
        sum += *(reinterpret_cast<const uint8_t*>(p) + len - 1);
    }
    for(int i = 0; i < len / 2; i++) {
        sum += *(p + i);
    }

    while (sum & 0xffff0000) {
        sum = (sum >> 16) + (sum & 0xffff);
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
    printf("\nPinging %s [%s]\n\n", hostname.c_str(), inet_ntoa(serv_addr.sin_addr));

    const std::string msg = "test packet";

    while (true) {
        sleep(1);
        struct sockaddr_in src_addr{};

        memset(data, 0, sizeof(icmphdr) + msg.size());
#ifdef WIN32
        icmp_hdr.sequence = sequence++;
#else
        icmp_hdr.un.echo.sequence = sequence++;
#endif
        icmp_hdr.checksum = 0;
        memcpy(data, &icmp_hdr, sizeof icmp_hdr); //add header
        memcpy(data + sizeof(icmp_hdr), msg.data(), msg.size());
        icmp_hdr.checksum = checksum(reinterpret_cast<unsigned short*>(data), sizeof(icmp_hdr) + msg.size());
        memcpy(data, &icmp_hdr, sizeof icmp_hdr);

        auto start = std::chrono::system_clock::now();
        ssize_t rc = sendto(sock_fd, data, sizeof(icmp_hdr) + msg.size(), 0, (struct sockaddr *) &serv_addr, sizeof serv_addr);
        if (rc <= 0) {
            fprintf(stderr, "Error occurred while sending data\n");
            return 1;
        }
        fd_set read_set;
        struct timeval timeout = {1, 0};
        memset(&read_set, 0, sizeof read_set);
        FD_SET(sock_fd, &read_set);
        rc = select(sock_fd + 1, &read_set, nullptr, nullptr, &timeout);
        if (rc == 0) {
            printf("No response from remote host\n");
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
        } else if (rc < sizeof(icmphdr)) {
            fprintf(stderr, "Error, got short ICMP packet, %zd bytes\n", rc);
            return 1;
        }

        memcpy(&src_addr.sin_addr.s_addr, data + 12, sizeof(src_addr.sin_addr.s_addr));

        auto response_hdr = *reinterpret_cast<icmphdr*>(data + 20);
        printf("response from: %s\n", inet_ntoa(src_addr.sin_addr));
        printf("  RTT: %.0f ms\n", elapsed_seconds.count() * 1000);
        printf("  Type: %d\n", response_hdr.type);
        printf("  Code: %d\n", response_hdr.code);
#ifdef WIN32
        printf("  Identifier: %d\n", response_hdr.id);
        printf("  Sequence: %d\n", response_hdr.sequence);
#else
        printf("  Identifier: %d\n", response_hdr.un.echo.id);
        printf("  Sequence: %d\n", response_hdr.un.echo.sequence);
#endif
        printf("  Data: %s\n", data + 20 + sizeof(icmphdr));
    }
}
