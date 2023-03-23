#ifndef SERVER_PARSE_HTTP_HPP
#define SERVER_PARSE_HTTP_HPP

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
    std::string first_line;
    http_header header;
    std::string body;
};

struct response {
    std::string first_line;
    http_header header;
    std::string body;
};

request parse_request(const std::string &s);
response parse_response(const std::string &s);
std::ostream& operator<<(std::ostream& os, const response& response);
std::ostream& operator<<(std::ostream& os, const request& request);
std::string to_string(const response& response);
std::string to_string(const request& request);
#endif //SERVER_PARSE_HTTP_HPP
