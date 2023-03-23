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
#include "../../parse_http/parse_http.hpp"
#include "../../read_http//read_http.hpp"
#include "../../open_tcp_connection/open_tcp_connection.hpp"

int main(int argc, char *argv[]) {
    const std::string output_filename = "response";

    if (argc < 5) {
        fprintf(stderr, "Too few arguments. Expected `hostname` `port` `target hostname` 'route'\n");
        return 1;
    }
    int sock_fd = open_tcp_connection(std::string(argv[1]), atoi(argv[2]));

    std::string target_hostname(argv[3]);
    std::string target_route(argv[4]);

    request request;
    std::string format = "GET %s HTTP/1.0";
    request.first_line = std::string(format.size() + target_route.size() - 2, 0);
    sprintf(request.first_line.data(), format.data(), target_route.data());
    request.header["Host"] = target_hostname;
    write_request(sock_fd, request);

    std::ofstream f(output_filename, std::ostream::binary);
    auto response = read_response(sock_fd);
    f.write(response.body.data(), response.body.size());
    f.close();
    close(sock_fd);
    return 0;
}