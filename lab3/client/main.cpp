#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>
#include <fstream>
#include <iostream>

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Too few arguments. Expected `hostname` `port` `file path`\n");
        exit(0);
    }

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        fprintf(stderr, "Error occurred while opening socket\n");
        exit(1);
    }
    hostent *host = gethostbyname(argv[1]);
    int port = atoi(argv[2]);
    if (host == nullptr) {
        fprintf(stderr, "Error: host not found\n");
        exit(0);
    }
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    bcopy((char *)host->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          host->h_length);
    serv_addr.sin_port = htons(port);
    if (connect(sock_fd, reinterpret_cast<sockaddr *>(&serv_addr), sizeof(serv_addr)) < 0) {
        fprintf(stderr, "Error occurred while connecting to socket\n");
        exit(1);
    }

    char *file_path = argv[3];
    char request[256];
    sprintf(request, "POST %s HTTP/1.1"
                            "Host: 127.0.0.1:8000"
                            "User-Agent: hw3 client"
                            "Accept: */*"
                            "Content-Length: 0", file_path);

    int n = write(sock_fd, request, strlen(request));
    if (n < 0) {
        fprintf(stderr, "Error occurred while writing\n");
        exit(0);
    }
    char response[200000];
    n = read(sock_fd, response, 200000);

    if (n < 0) {
        fprintf(stderr, "Error occurred while reading\n");
        exit(0);
    }
    char *ptr = response;
    while(n > 0) {
        ptr += n;
        n = read(sock_fd, ptr, 200000 - (ptr - response));
    }
    n = ptr - response;
    std::cout << n << std::endl;

    char *a = file_path, *b = strtok(file_path, "/");
    while(b != nullptr) {
        a = b;
        b = strtok(nullptr, "/");
    }

    char *res = response;
    for(int i = 0; i < 3; i++) {
        while (*res != '\r') {
            ++res;
        }
        ++res;
    }
    ++res;
    std::ofstream f(a, std::ostream::binary);

    f.write(res, n - (res - response));
    f.close();
    close(sock_fd);
    return 0;
}
