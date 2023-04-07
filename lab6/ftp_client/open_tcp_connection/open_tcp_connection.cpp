#include "open_tcp_connection.hpp"

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>


int open_tcp_connection(std::string hostname, long port) {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        fprintf(stderr, "Error occurred while opening socket\n");
        exit(1);
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
    serv_addr.sin_port = htons(port);
    if (connect(sock_fd, reinterpret_cast<sockaddr *>(&serv_addr), sizeof(serv_addr)) < 0) {
        fprintf(stderr, "Error occurred while connecting to socket\n");
        exit(1);
    }
    return sock_fd;
}