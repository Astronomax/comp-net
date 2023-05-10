#include <iostream>
#include <string>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <netdb.h>
#include <vector>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Too few arguments. Expected {url} {payload_size}`\n");
        return 1;
    }

    std::string hostname(argv[1]);
    size_t payload_size = stoi(std::string(argv[2]));
    int sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);

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
    icmp_hdr.type = ICMP_ECHO;
    icmp_hdr.un.echo.id = 1234;


    int sequence = 0;
    char data[2048];
    //std::vector<char> data(sizeof icmp_hdr + payload_size);

    //int ttl = 1;
    //setsockopt(sock_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));


    const int MAX_HOPS_NUMBER = 50;
    for (int ttl = 1; ttl < MAX_HOPS_NUMBER; ttl++) {
        setsockopt(sock_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

        icmp_hdr.un.echo.sequence = sequence++;
        memcpy(data, &icmp_hdr, sizeof icmp_hdr); //add header

        //ssize_t rc = sendto(sock_fd, data.data(), data.size(), 0, (struct sockaddr*)&serv_addr, sizeof serv_addr);
        ssize_t rc = sendto(sock_fd, data, sizeof icmp_hdr + payload_size, 0, (struct sockaddr*)&serv_addr, sizeof serv_addr);
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



        //we don't care about the sender address in this example..
        socklen_t slen = 0;
        struct icmphdr rcv_hdr{};
        //rc = recvfrom(sock_fd, data.data(), data.size(), 0, nullptr, &slen);
        rc = recvfrom(sock_fd, data, sizeof data, 0, nullptr, &slen);

        if (rc <= 0) {
            fprintf(stderr, "Error occurred while receiving data\n");
            break;
        } else if (rc < sizeof rcv_hdr) {
            printf("Error, got short ICMP packet, %zd bytes\n", rc);
            break;
        }
        memcpy(&rcv_hdr, data, sizeof rcv_hdr);
        if (rcv_hdr.type == ICMP_ECHOREPLY) {
            printf("%s reached in %d hops\n", hostname.c_str(), ttl);
            break;
        } else if (rcv_hdr.type == ICMP_TIME_EXCEEDED) {
            printf("hop %d: response from %s\n", ttl, hostname.c_str());
        } else {
            printf("Got ICMP packet with type 0x%x ?!?\n", rcv_hdr.type);
        }
    }
    return 0;
}
