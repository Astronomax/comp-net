#ifndef SERVER_MY_HTTP_HPP
#define SERVER_MY_HTTP_HPP

#include <cstdlib>
#include <iterator>
#include <type_traits>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <map>

typedef std::map<std::string, std::string> http_header;

struct request {
    std::string method;
    std::string path;
    std::string http_ver;
    http_header header;
    std::string body;
};

struct response {
    std::string http_ver;
    size_t status_code;
    std::string msg;
    http_header header;
    std::string body;
};

request parse_request(const std::string &s);
response parse_response(const std::string &s);
std::ostream& operator<<(std::ostream& os, const response& response);
std::ostream& operator<<(std::ostream& os, const request& request);
std::string to_string(const response& response);
std::string to_string(const request& request);
response serialize_response_from_file(const std::string &filename);
void deserialize_response_to_file(const std::string &filename, const response &response);
request read_request(int sock_fd);
response read_response(int sock_fd);
void write_request(int sock_fd, const request& request);
void write_response(int sock_fd, const response& response);

#endif //SERVER_MY_HTTP_HPP
