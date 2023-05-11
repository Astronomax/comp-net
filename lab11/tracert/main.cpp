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

    /*if (sock_fd < 0) {
        fprintf(stderr, "Error occurred while opening socket\n");
        return 1;
    }*/

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
    //std::vector<char> data(sizeof icmp_hdr + payload_size);

    //int ttl = 1;
    //setsockopt(sock_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));


    const int MAX_HOPS_NUMBER = 50;
    for (int ttl = 1; ttl < MAX_HOPS_NUMBER; ttl++) {
        //setsockopt(sock_fd, IPPROTO_IP, IP_TTL, reinterpret_cast<char*>(&ttl), sizeof ttl);

        memset(data, 0, sizeof(icmp_hdr) + payload_size);
        icmp_hdr.sequence = sequence++;
        icmp_hdr.checksum = 0;
        memcpy(data, &icmp_hdr, sizeof icmp_hdr); //add header
        //for(int j = 0; j < sizeof(icmp_hdr) + payload_size; j++) {
        //    printf("%x", data[j]);
        //}
        //printf("\n");
        //*reinterpret_cast<int8_t*>(data + 8) = 20;


        icmp_hdr.checksum = checksum(reinterpret_cast<unsigned short*>(data), sizeof(icmp_hdr) + payload_size);

        //printf("sum: %x\n", icmp_hdr.checksum);

        memcpy(data, &icmp_hdr, sizeof icmp_hdr); //add header


        setsockopt(sock_fd, IPPROTO_IPV6, IP_TTL, (char*)&ttl, sizeof(ttl));



        //ssize_t rc = sendto(sock_fd, data.data(), data.size(), 0, (struct sockaddr*)&serv_addr, sizeof serv_addr);
        ssize_t rc = sendto(sock_fd, data, sizeof(icmp_hdr) + payload_size, 0, (struct sockaddr*)&serv_addr, sizeof serv_addr);
        if (rc <= 0) {
            fprintf(stderr, "Error occurred while sending data\n");
            return 1;
        }
        std::cout << "ICMP sent" << std::endl;





        /*
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
        if (rcv_hdr.type == 0) {
            printf("%s reached in %d hops\n", hostname.c_str(), ttl);
            break;
        } else if (rcv_hdr.type == 11) {
            printf("hop %d: response from %s\n", ttl, hostname.c_str());
        } else {
            printf("Got ICMP packet with type 0x%x ?!?\n", rcv_hdr.type);
        }*/

//        struct iphdr ip_response_header{};


        fd_set fdr;
        FD_ZERO(&fdr);
        FD_SET(sock_fd, &fdr);
        timeval timeout{};
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        std::string res;
        data_batch batch{};
        int rrc;
        do {
            batch.data_len = recv(sock_fd, batch.data, data_batch::BUFFER_SIZE, 0);
            if(batch.data_len <= 0) break;
            res += std::string(batch.data, batch.data_len);
            rrc = select(0, &fdr, nullptr, nullptr, &timeout);
        } while(rrc);

        /*
        struct timeval tv{};
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        setsockopt(sock_fd, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char*>(&tv), sizeof(tv));
        setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&tv), sizeof(tv));

        auto data_length_byte = recv(sock_fd, reinterpret_cast<char*>(&data), sizeof(data), 0);
        */
        /*if (data_length_byte == -1) {
            std::cout << ttl << "\033[1;35m" << " * * *" << "\033[0m" << std::endl;
            continue;
        }*/

        auto data_length_byte = res.size();

        struct sockaddr_in src_addr{};
        int8_t type = *reinterpret_cast<int8_t*>(res.data() + 20);
        memcpy(&src_addr.sin_addr.s_addr, res.data() + 12, sizeof src_addr.sin_addr.s_addr);

        std::cout << ttl << " " << "\033[1;35m" <<  inet_ntoa(src_addr.sin_addr) << "\033[0m" << std::endl;

        if (type == 0) {
            std::cout << std::endl << "\033[1;35m" << ttl << "\033[0m" << " hops between you and " << hostname.c_str() << std::endl;
            break;
        }
    }
    return 0;
}
