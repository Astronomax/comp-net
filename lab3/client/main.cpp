#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Too few arguments. Expected `hostname` and `port`\n");
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

    printf("Please enter the message for server: ");
    char buffer[256];
    bzero(buffer, 256);
    fgets(buffer, 255, stdin);

    int n = write(sock_fd, buffer, strlen(buffer));
    if (n < 0) {
        fprintf(stderr, "Error occurred while writing\n");
        exit(0);
    }
    bzero(buffer, 256);
    n = read(sock_fd, buffer, 255);
    if (n < 0) {
        fprintf(stderr, "Error occurred while reading\n");
        exit(0);
    }
    printf("Message from server: %s\n", buffer);
    close(sock_fd);
    return 0;
}
