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

struct icmphdr
{
    uint8_t type;                /* message type */
    uint8_t code;                /* type sub-code */
    uint16_t checksum;
    uint16_t id;
    uint16_t sequence;
};

uint16_t checksum(const void *data, size_t len) {
    auto p = reinterpret_cast<const uint16_t*>(data);

    uint32_t sum = 0;

    //if (len & 1) {
    //    sum = reinterpret_cast<const uint8_t *>(p)[len - 1];
    //}

    //len /= 2;
    if (len % 2 == 1) {
        assert(false);
    }
    for(int i = 0; i < len / 2; i++) {
        sum += *(p + i);
    }

    //if (sum & 0xffff0000) {
    //    sum = (sum >> 16) + (sum & 0xffff);
    //}

    return static_cast<uint16_t>(~sum);
}


unsigned short checksum1(unsigned short* buffer, int size) {
    unsigned short sum = 0;
    unsigned short cnt = 0;
    unsigned short cnt_l = 0;
    unsigned short last_sum = 0;
    unsigned long long total_sum = 0;
    while(size > 1) {
        total_sum += *buffer;
        sum += *buffer++;
        last_sum = sum;
        size -= sizeof(unsigned short);
    }
    if(size) {
        sum += *buffer;
        total_sum += *buffer;
    }
    cnt = total_sum / 0xffff;
    sum += cnt;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    sum = ~sum;
    return sum;
}

struct data_batch {
    data_batch() = default;
    static const size_t BUFFER_SIZE = 2048;
    char data[BUFFER_SIZE];
    ssize_t data_len;
};




int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Too few arguments. Expected {url} {payload_size}`\n");
        return 1;
    }

#ifdef WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    std::string hostname(argv[1]);
    size_t payload_size = stoi(std::string(argv[2]));
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
    memcpy((char*)&serv_addr.sin_addr.s_addr,
           (char*)host->h_addr,
           host->h_length);

    struct icmphdr icmp_hdr{};
    memset(&icmp_hdr, 0, sizeof icmp_hdr);
    icmp_hdr.type = 8;
    icmp_hdr.id = 1234;

    uint16_t sequence = 1;
    char data[2048];

    const int MAX_HOPS_NUMBER = 50;
    for (int ttl = 1; ttl < MAX_HOPS_NUMBER; ttl++) {
        memset(data, 0, sizeof(icmp_hdr) + payload_size);
        icmp_hdr.sequence = sequence++;
        icmp_hdr.checksum = 0;
        memcpy(data, &icmp_hdr, sizeof icmp_hdr); //add header

        icmp_hdr.checksum = checksum(reinterpret_cast<unsigned short*>(data), sizeof(icmp_hdr) + payload_size);

        memcpy(data, &icmp_hdr, sizeof icmp_hdr); //add header


        setsockopt(sock_fd, IPPROTO_IPV6, IP_TTL, (char*)&ttl, sizeof(ttl));



        ssize_t rc = sendto(sock_fd, data, sizeof(icmp_hdr) + payload_size, 0, (struct sockaddr*)&serv_addr, sizeof serv_addr);
        if (rc <= 0) {
            fprintf(stderr, "Error occurred while sending data\n");
            return 1;
        }
        std::cout << "ICMP sent" << std::endl;






        fd_set read_set;
        struct timeval timeout = {2, 0};
        memset(&read_set, 0, sizeof read_set);
        FD_SET(sock_fd, &read_set);
        rc = select(sock_fd + 1, &read_set, nullptr, nullptr, &timeout);
        if (rc == 0) {
            std::cout << "Got no reply" << std::endl;
            continue;
        } else if (rc < 0) {
            fprintf(stderr, "Error occurred while receiving data\n");
            break;
        }

        socklen_t slen = 0;
        rc = recv(sock_fd, data, sizeof data, 0);
        if (rc <= 0) {
            fprintf(stderr, "Error occurred while receiving data\n");
            break;
        } else if (rc < sizeof icmp_hdr) {
            printf("Error, got short ICMP packet, %zd bytes\n", rc);
            break;
        }

        struct sockaddr_in src_addr{};
        int8_t type = *reinterpret_cast<int8_t*>(data + 20);
        memcpy(&src_addr.sin_addr.s_addr, data + 12, sizeof src_addr.sin_addr.s_addr);

        std::cout << ttl << " " << "\033[1;35m" <<  inet_ntoa(src_addr.sin_addr) << "\033[0m" << std::endl;

        if (type == 0) {
            std::cout << std::endl << "\033[1;35m" << ttl << "\033[0m" << " hops between you and " << hostname.c_str() << std::endl;
            break;
        }
    }
    return 0;
}
