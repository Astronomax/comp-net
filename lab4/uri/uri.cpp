#include "uri.hpp"

uri uri::Parse(const std::string &uri) {
    class uri result;
    if (uri.empty()) {
        return result;
    }
    iterator_t uri_end = uri.end();
    iterator_t query_start = std::find(uri.begin(), uri_end, '?');
    iterator_t protocol_start = uri.begin();
    iterator_t protocol_end = std::find(protocol_start, uri_end, ':');

    if (protocol_end != uri_end) {
        std::string prot = &*(protocol_end);
        if ((prot.length() > 3) && (prot.substr(0, 3) == "://")) {
            result.protocol = std::string(protocol_start, protocol_end);
            protocol_end += 3;
        } else {
            protocol_end = uri.begin();
        }
    } else {
        protocol_end = uri.begin();
    }

    iterator_t host_start = protocol_end;
    iterator_t path_start = std::find(host_start, uri_end, '/');
    iterator_t host_end = std::find(protocol_end, (path_start != uri_end) ? path_start : query_start, ':');

    result.host = std::string(host_start, host_end);

    if ((host_end != uri_end) && ((&*(host_end))[0] == ':')) {
        host_end++;
        iterator_t portEnd = (path_start != uri_end) ? path_start : query_start;
        result.port = std::string(host_end, portEnd);
    }

    if (path_start != uri_end) {
        result.path = std::string(path_start, query_start);
    }

    if (query_start != uri_end) {
        result.query = std::string(query_start, uri.end());
    }
    return result;
}