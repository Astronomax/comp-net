#include "my_http.hpp"
#include <sstream>
#include <fstream>
#include "../data_batch/data_batch.hpp"

static const std::string CRLF = "\r\n";

std::string_view parse_token(std::string_view &view) {
    ssize_t space_pos = view.find(' ');
    if (space_pos == -1) {
        std::string_view token = view;
        view = std::string_view();
        return token;
    } else {
        std::string_view token = view.substr(0, space_pos);
        size_t next_line_pos = space_pos + 1;
        view = view.substr(next_line_pos, view.size() - next_line_pos);
        return token;
    }
};

std::string_view parse_line(std::string_view &view) {
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
        auto line = parse_line(view);
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
    request request;
    std::string_view view(s);

    auto first_line = parse_line(view);
    request.method = std::string(parse_token(first_line));
    request.path = std::string(parse_token(first_line));
    request.http_ver = std::string(parse_token(first_line));

    request.header = parse_header(view);
    request.body = std::string(view);
    return request;
}

response parse_response(const std::string &s) {
    response response;
    std::string_view view(s);

    auto first_line = parse_line(view);
    response.http_ver = std::string(parse_token(first_line));
    response.status_code = stoi(std::string(parse_token(first_line)));
    response.msg = std::string(parse_token(first_line));

    response.header = parse_header(view);
    response.body = std::string(view);
    return response;
}

std::ostream& operator<<(std::ostream& os, const response& response) {
    os << response.http_ver + " " + std::to_string(response.status_code) + " " + response.msg + CRLF;
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
    os << request.method + " " + request.path + " " + request.http_ver + CRLF;
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