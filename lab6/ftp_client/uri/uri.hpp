#ifndef SERVER_URI_HPP
#define SERVER_URI_HPP

#include <string>
#include <algorithm>

class uri {
public:
    static uri Parse(const std::string &uri);
    std::string query, path, protocol, host, port;

private:
    typedef std::string::const_iterator iterator_t;
};


#endif //SERVER_URI_HPP
