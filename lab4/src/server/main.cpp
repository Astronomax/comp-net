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
#include <string>
#include <sys/stat.h>
#include "../../my_http/my_http.hpp"
#include "../../open_tcp_connection/open_tcp_connection.hpp"

struct session_routine_args {
    int sock_fd;
    std::atomic_int &accepted_connections;
};

inline bool file_exists (const std::string& name)  {
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}

void log_status(std::string hostname, int status) {
    std::ofstream log("log.txt", std::ios::app);
    log << hostname << ": " << status << "\n";
    log.close();
}

std::vector<std::string> read_blacklist() {
    std::vector<std::string> blacklist;
    std::ifstream f("blacklist", std::ostream::binary);
    while(!f.eof()) {
        std::string hostname;
        f >> hostname;
        blacklist.push_back(hostname);
    }
    f.close();
    return blacklist;
}

void* session_routine(void* args_) {
    auto args = reinterpret_cast<session_routine_args*>(args_);
    auto blacklist = read_blacklist();

    auto request = read_request(args->sock_fd);
    std::string hostname = request.header["Host"];

    if(std::find(blacklist.begin(), blacklist.end(), hostname) != blacklist.end()) {
        response response;
        response.http_ver = "HTTP/1.1";
        response.status_code = 403;
        response.msg = "Forbidden";
        log_status(hostname, 403);
        write_response(args->sock_fd, response);
        return new int(0);
    }

    std::hash<std::string> hasher;
    std::string hash = std::to_string(hasher(to_string(request)));
    int sock_fd = open_tcp_connection(hostname, 80);
    if (!file_exists("cache_" + hash)) {
        write_request(sock_fd, request);
        auto response = read_response(sock_fd);
        auto last_modified = response.header.find("Last-Modified");
        auto etag = response.header.find("ETag");
        if (last_modified != response.header.end() &&
            etag != response.header.end()) {
            deserialize_response_to_file("cache_" + hash, response);
        }
        log_status(hostname, response.status_code);
        write_response(args->sock_fd, response);
    } else {
        auto cached_response = serialize_response_from_file("cache_" + hash);
        auto last_modified = cached_response.header["Last-Modified"];
        auto etag = cached_response.header["ETag"];
        request.header["If-Modified-Since"] = last_modified;
        request.header["If-None-Match"] = etag;
        write_request(sock_fd, request);
        auto response = read_response(sock_fd);

        if(response.status_code == 304) {
            log_status(hostname, cached_response.status_code);
            write_response(args->sock_fd, cached_response);
        } else {
            log_status(hostname, response.status_code);
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
