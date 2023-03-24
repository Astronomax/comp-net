#include <cstdio>
#include <unistd.h>
#include <string>
#include "../../my_http/my_http.hpp"
#include "../../open_tcp_connection/open_tcp_connection.hpp"
#include "../../uri/uri.hpp"

int main(int argc, char *argv[]) {
    const std::string output_filename = "response";

    if (argc < 2) {
        fprintf(stderr, "Too few arguments. Expected {uri}`\n");
        return 1;
    }

    uri uri = uri::Parse(argv[1]);

    int sock_fd = open_tcp_connection(uri.host, (uri.port.empty() ? 80 : stoi(uri.port)));

    request request;
    request.method = "GET";
    request.path = uri.path + uri.query;
    request.http_ver = "HTTP/1.0";
    request.header["Host"] = uri.host;
    write_request(sock_fd, request);

    auto response = read_response(sock_fd);
    deserialize_response_to_file(output_filename, response);
    close(sock_fd);
    return 0;
}