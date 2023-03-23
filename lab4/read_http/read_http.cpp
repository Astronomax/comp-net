#include "read_http.hpp"
#include "../data_batch/data_batch.hpp"
#include "../parse_http/parse_http.hpp"

std::string read_data(int sock_fd) {
    std::string data;
    data_batch batch;
    fcntl(sock_fd, F_SETFL, (fcntl(sock_fd, F_GETFL, 0) & ~O_NONBLOCK));
    batch.data_len = read(sock_fd, batch.data, data_batch::BUFFER_SIZE);
    fcntl(sock_fd, F_SETFL, (fcntl(sock_fd, F_GETFL, 0) | O_NONBLOCK));
    while(batch.data_len > 0) {
        data += std::string(batch.data, batch.data_len);
        batch.data_len = read(sock_fd, batch.data, data_batch::BUFFER_SIZE);
    }
    return data;
}

request read_request(int sock_fd) {
    return parse_request(read_data(sock_fd));
}
response read_response(int sock_fd) {
    return parse_response(read_data(sock_fd));
}
void write_request(int sock_fd, const request& request) {
    std::string data = to_string(request);
    send(sock_fd, data.data(), data.size(), 0);
}
void write_response(int sock_fd, const response& response) {
    std::string data = to_string(response);
    send(sock_fd, data.data(), data.size(), 0);
}