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
#include "../../socket_read/socket_read.hpp"
#include "../../open_tcp_connection/open_tcp_connection.hpp"
#include <unordered_map>
#include <string>
#include <sys/stat.h>

struct session_routine_args {
    int sock_fd;
    std::atomic_int &accepted_connections;
};

inline bool file_exists (const std::string& name)  {
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}

std::string parse_host(const std::string& request) {
    std::string_view view(request);
    ssize_t pos = view.find(' ');
    view = view.substr(pos + 1, view.size() - pos - 1);
    pos = view.find(' ');
    view = view.substr(0, pos);
    return std::string(view);
}

void* session_routine(void* args_) {
    auto args = reinterpret_cast<session_routine_args*>(args_);

    const size_t BUFFER_SIZE = 2048;
    std::string buffer(BUFFER_SIZE, 0);

    std::string request;
    fcntl(args->sock_fd, F_SETFL, (fcntl(args->sock_fd, F_GETFL, 0) & ~O_NONBLOCK));
    ssize_t n = read(args->sock_fd, buffer.data(), data_batch::BUFFER_SIZE);
    fcntl(args->sock_fd, F_SETFL, (fcntl(args->sock_fd, F_GETFL, 0) | O_NONBLOCK));
    while(n > 0) {
        request += buffer.substr(0, n);
        n = read(args->sock_fd, buffer.data(), data_batch::BUFFER_SIZE);
    }

    std::hash<std::string> hasher;
    std::string hash = std::to_string(hasher(request));
    std::string hostname = parse_host(request);
    int sock_fd = open_tcp_connection(hostname, 80);

    if(!file_exists(hash)) {
        n = send(sock_fd, request.data(), request.size(), 0);
        if (n < 0) {
            fprintf(stderr, "Error occurred while sending \n");
            return new int(1);
        }

        fcntl(sock_fd, F_SETFL, (fcntl(sock_fd, F_GETFL, 0) & ~O_NONBLOCK));
        n = read(sock_fd, buffer.data(), data_batch::BUFFER_SIZE);
        fcntl(sock_fd, F_SETFL, (fcntl(sock_fd, F_GETFL, 0) | O_NONBLOCK));

        std::ofstream f(hash, std::ostream::binary);
        while(n > 0) {
            f.write(buffer.data(), n);
            n = read(sock_fd, buffer.data(), data_batch::BUFFER_SIZE);
        }
        f.close();
    } else {
        //check if file is fresh
        //update if needs
    }

    std::ifstream f(hash, std::ostream::binary);
    n = f.readsome(buffer.data(), buffer.size());
    while(n > 0) {
        n = send(args->sock_fd, buffer.data(), n, 0);
        if (n < 0) {
            fprintf(stderr, "Error occurred while sending \n");
            return new int(1);
        }
        n = f.readsome(buffer.data(), buffer.size());
    }
    f.close();

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
