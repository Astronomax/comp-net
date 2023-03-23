#include <sstream>
#include <fstream>
#include "parse_http.hpp"
#include "../data_batch/data_batch.hpp"

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
        if(!line.empty()) {
            size_t delimiter_pos = line.find(delimiter);
            std::string field_name = std::string(line.substr(0, delimiter_pos));
            std::string value = std::string(line.substr(delimiter_pos + delimiter.size()));
            header[field_name] = value;
        } else break; //body
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

response serialize_response_from_file(const std::string &filename) {
    std::string response_str;
    data_batch batch;
    std::ifstream f(filename, std::ostream::binary);
    ssize_t n = f.readsome(batch.data, data_batch::BUFFER_SIZE);
    while(n > 0) {
        response_str += std::string(batch.data, n);
        n = f.readsome(batch.data, data_batch::BUFFER_SIZE);
    }
    f.close();
    return parse_response(response_str);
}

void deserialize_response_to_file(const std::string &filename, const response &response) {
    auto response_str = to_string(response);
    std::ofstream f(filename, std::ostream::binary);
    f.write(response_str.data(), response_str.size());
    f.close();
}