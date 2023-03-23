#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>
#include <fstream>
#include <iostream>
#include "../../socket_read/socket_read.hpp"
#include "../../open_tcp_connection/open_tcp_connection.hpp"

int main(int argc, char *argv[]) {
    const std::string output_filename = "response";

    if (argc < 4) {
        fprintf(stderr, "Too few arguments. Expected `hostname` `port` `target hostname`\n");
        return 1;
    }
    int sock_fd = open_tcp_connection(std::string(argv[1]), atoi(argv[2]));

    std::string target_hostname(argv[3]);
    std::string format = "GET %s HTTP/1.0\r\n\r\n";
    std::string request(format.size() + target_hostname.size() - 2, 0);
    sprintf(request.data(), format.data(), target_hostname.data());

    ssize_t n = write(sock_fd, request.data(), request.size());
    if (n < 0) {
        fprintf(stderr, "Error occurred while writing\n");
        return 1;
    }

    std::ofstream f(output_filename, std::ostream::binary);
    for (auto i : socket_read(sock_fd)) {
        if(i.first == SOCK_READ_STATE::SRS_BODY) {
            f.write(i.second.data(), i.second.size());
        }
    }
    f.close();
    close(sock_fd);
    return 0;
}