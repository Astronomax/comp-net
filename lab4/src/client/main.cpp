#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <string>
#include "../../my_http/my_http.hpp"
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
    request.method = "GET";
    request.path = target_route;
    request.http_ver = "HTTP/1.0";
    request.header["Host"] = target_hostname;
    write_request(sock_fd, request);

    auto response = read_response(sock_fd);
    deserialize_response_to_file(output_filename, response);
    close(sock_fd);
    return 0;
}