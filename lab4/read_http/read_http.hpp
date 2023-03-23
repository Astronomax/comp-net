#ifndef SERVER_READ_HTTP_HPP
#define SERVER_READ_HTTP_HPP

#include "../parse_http/parse_http.hpp"

request read_request(int sock_fd);
response read_response(int sock_fd);
void write_request(int sock_fd, const request& request);
void write_response(int sock_fd, const response& response);

#endif //SERVER_READ_HTTP_HPP
