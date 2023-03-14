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

struct session_routine_args {
    int sock_fd;
    std::atomic_int &accepted_connections;
};

bool ends_with(const std::string &s, const std::string &t) {
    return (s.substr(s.size() - t.size(), t.size()) == t);
}

std::string contentType(const std::string &file_name) {
    if (ends_with(file_name, ".htm") || ends_with(file_name, ".html")) {
        return "text/html";
    } else if (ends_with(file_name, ".ra") || ends_with(file_name, ".ram")) {
        return "audio/x-pn-realaudio";
    }
    return "application/octet-stream";
}

void* session_routine(void* args_) {
    auto args = reinterpret_cast<session_routine_args*>(args_);
    char buffer[4096];
    bzero(buffer, 4096);
    ssize_t n = read(args->sock_fd, buffer, 4095);

    if (n < 0) {
        fprintf(stderr, "Error occurred while reading\n");
        return nullptr;
    }

    const std::string CRLF = "\r\n";
    std::string response;
    strtok(buffer, " ");
    std::string file_name = std::string(strtok(nullptr, " "));
    file_name = "." + file_name;

    try {
        std::ifstream file(file_name);
        file.seekg(0, std::ios::end);
        std::streamsize file_size = file.tellg();
        char *file_bytes = new char [file_size];
        file.seekg(0);
        file.read(file_bytes, file_size);
        file.close();
        std::string statusLine = "HTTP/1.0 200 OK" + CRLF;
        std::string contentTypeLine = "Content-Type: " + contentType(file_name) + CRLF;
        std::string fileContent(file_size, 0);
        std::copy(file_bytes, file_bytes + file_size, fileContent.begin());
        response = statusLine + contentTypeLine + CRLF + fileContent;
        delete [] file_bytes;
    } catch (std::exception &e) {
        std::string statusLine = "HTTP/1.0 404 Not Found" + CRLF;
        std::string contentTypeLine = "Content-Type: text/html" + CRLF;
        std::string entityBody = "<HTML>"
                                 "<HEAD><TITLE>Not Found</TITLE></HEAD>"
                                 "<BODY>Not Found</BODY></HTML>";
        response = statusLine + contentTypeLine + CRLF + entityBody;
    }
    n = send(args->sock_fd, response.data(), response.size(), 0);
    if (n < 0) {
        fprintf(stderr, "Error occurred while sending \n");
        return nullptr;
    }
    close(args->sock_fd);
    --args->accepted_connections;
    return nullptr;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Error - no port provided\n");
        exit(1);
    }
    int port = atoi(argv[1]);
    int passive_sock_fd =  socket(AF_INET, SOCK_STREAM, 0);
    if (passive_sock_fd < 0) {
        fprintf(stderr, "Error occurred while opening socket\n");
        exit(1);
    }
    sockaddr_in serv_addr {};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    if (bind(passive_sock_fd, reinterpret_cast<sockaddr *>(&serv_addr), sizeof(serv_addr)) < 0) {
        fprintf(stderr, "Error occurred while binding socket\n");
        exit(1);
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
