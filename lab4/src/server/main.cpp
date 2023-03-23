#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <fstream>
#include <pthread.h>
#include <vector>
#include <algorithm>
#include <atomic>
#include <fcntl.h>
#include "../../parse_http/parse_http.hpp"
#include "../../open_tcp_connection/open_tcp_connection.hpp"
#include "../../data_batch/data_batch.hpp"
#include <string>
#include <sys/stat.h>
#include "../../read_http/read_http.hpp"

struct session_routine_args {
    int sock_fd;
    std::atomic_int &accepted_connections;
};

inline bool file_exists (const std::string& name)  {
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
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

int parse_status(const response &response) {
    std::string_view view(response.first_line);
    size_t pos = view.find(' ');
    view = view.substr(pos + 1, view.size() - pos - 1);
    view = view.substr(0, view.find(' '));
    return stoi(std::string(view));
}

void log_status(int status) {
    std::ofstream log("log.txt", std::ios::app);
    log << status << "\n";
    log.close();
}

void* session_routine(void* args_) {
    auto args = reinterpret_cast<session_routine_args*>(args_);

    auto request = read_request(args->sock_fd);

    std::hash<std::string> hasher;
    std::string hash = std::to_string(hasher(to_string(request)));
    int sock_fd = open_tcp_connection(request.header["Host"], 80);

    if (!file_exists("cache_" + hash)) {
        write_request(sock_fd, request);
        auto response = read_response(sock_fd);
        auto last_modified = response.header.find("Last-Modified");
        auto etag = response.header.find("ETag");
        if (last_modified != response.header.end() &&
            etag != response.header.end()) {
            deserialize_response_to_file("cache_" + hash, response);
        }
        log_status(parse_status(response));
        write_response(args->sock_fd, response);
    } else {
        auto cached_response = serialize_response_from_file("cache_" + hash);
        auto last_modified = cached_response.header["Last-Modified"];
        auto etag = cached_response.header["ETag"];
        request.header["If-Modified-Since"] = last_modified;
        request.header["If-None-Match"] = etag;
        write_request(sock_fd, request);
        auto response = read_response(sock_fd);
        int status = parse_status(response);
        if(status == 304) {
            log_status(parse_status(cached_response));
            write_response(args->sock_fd, cached_response);
        } else {
            log_status(parse_status(response));
            write_response(args->sock_fd, response);
        }
    }

    close(args->sock_fd);
    close(sock_fd);
    --args->accepted_connections;
    return new int(0);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Error - no port provided\n");
        return 1;
    }
    int port = atoi(argv[1]);
    int passive_sock_fd =  socket(AF_INET, SOCK_STREAM, 0);
    if (passive_sock_fd < 0) {
        fprintf(stderr, "Error occurred while opening socket\n");
        return 1;
    }
    sockaddr_in serv_addr {};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    if (bind(passive_sock_fd, reinterpret_cast<sockaddr *>(&serv_addr), sizeof(serv_addr)) < 0) {
        fprintf(stderr, "Error occurred while binding socket\n");
        return 1;
    }
    listen(passive_sock_fd, 0);
    std::atomic_int accepted_connections = 0;
    printf("Waiting for clients...\n");
    while(true) {
        if (accepted_connections == 5) continue;
        sockaddr_in cli_addr{};
        socklen_t cli_len = sizeof(cli_addr);
        int active_sock_fd = accept(passive_sock_fd, reinterpret_cast<sockaddr *>(&cli_addr), &cli_len);
        if (active_sock_fd < 0) {
            printf("Error occurred while accepting incoming connection\n");
            continue;
        }
        ++accepted_connections;
        printf("Got connection from %s port %d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
        pthread_t new_thread;
        pthread_create(&new_thread, nullptr, session_routine, new session_routine_args {active_sock_fd, accepted_connections});
    }
}
