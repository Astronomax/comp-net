#include <sstream>
#include "parse_http.hpp"

static const std::string CRLF = "\r\n";

std::string_view read_line(std::string_view &view) {
    size_t CRLF_pos = view.find(CRLF);
    std::string_view line = view.substr(0, CRLF_pos);
    size_t next_line_pos = CRLF_pos + CRLF.size();
    view = view.substr(next_line_pos, view.size() - next_line_pos);
    return line;
};

http_header parse_header(std::string_view &view) {
    const std::string delimiter = ": ";
    http_header header;
    while(!view.empty()) {
        auto line = read_line(view);
        size_t delimiter_pos = line.find(delimiter);
        std::string field_name = std::string(line.substr(0, delimiter_pos));
        std::string value = std::string(line.substr(delimiter_pos + delimiter.size()));
        header[field_name] = value;
        if(view.find(CRLF) == 0) {
            view = view.substr(2, view.size() + 2);
            break; //body
        }
    }
    return header;
}

request parse_request(const std::string &s) {
    request response;
    std::string_view view(s);
    response.first_line = std::string(read_line(view));
    response.header = parse_header(view);
    response.body = std::string(view);
    return response;
}

response parse_response(const std::string &s) {
    response response;
    std::string_view view(s);
    response.first_line = std::string(read_line(view));
    response.header = parse_header(view);
    response.body = std::string(view);
    return response;
}

std::ostream& operator<<(std::ostream& os, const response& response) {
    os << response.first_line + CRLF;
    std::string format = "%s: %s";
    for(auto &entry : response.header) {
        std::string entry_line(format.size() + entry.first.size() + entry.second.size() - 4, 0);
        sprintf(entry_line.data(), format.data(), entry.first.data(), entry.second.data());
        entry_line += CRLF;
        os << entry_line;
    }
    os << CRLF;
    os << response.body;
    return os;
}

std::ostream& operator<<(std::ostream& os, const request& request) {
    os << request.first_line + CRLF;
    std::string format = "%s: %s";
    for(auto &entry : request.header) {
        std::string entry_line(format.size() + entry.first.size() + entry.second.size() - 4, 0);
        sprintf(entry_line.data(), format.data(), entry.first.data(), entry.second.data());
        entry_line += CRLF;
        os << entry_line;
    }
    os << CRLF;
    os << request.body;
    return os;
}

std::string to_string(const response& response) {
    std::ostringstream ss;
    ss << response;
    return ss.str();
}

std::string to_string(const request& request) {
    std::ostringstream ss;
    ss << request;
    return ss.str();
}