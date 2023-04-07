#include <winsock2.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "uri/uri.hpp"
#include <utility>
#include <strings.h>
#include "open_tcp_connection/open_tcp_connection.hpp"

std::string readServ(int s) {
    fd_set fdr;
    FD_ZERO(&fdr);
    FD_SET(s,&fdr);
    timeval timeout{};
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    std::string res;
    char buff[512];
    int rc;
    do {
        std::fill(std::begin(buff), std::end(buff), 512);
        int n = recv(s, buff,511,0);
        if(n <= 0) break;
        res += std::string(buff);
        rc = select(0, &fdr,nullptr,nullptr, &timeout);
    } while(rc);
    return res;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Too few arguments. Expected {uri}`\n");
        return 1;
    }

    uri uri = uri::Parse(argv[1]);
    if (uri.protocol != "ftp") {
        fprintf(stderr, "Expected ftp protocol`\n");
        return 1;
    }

#ifdef WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    // open control channel
    int sock_fd = open_tcp_connection(uri.host, (uri.port.empty() ? 21 : stoi(uri.port)));
    std::cout << readServ(sock_fd) << std::endl;
    // open control channel

    // authorization
    std::string name = "TestUser";
    char str[512];
    sprintf(str,"USER %s\r\n", name.data());
    send(sock_fd, str, (int)strlen(str),0);
    std::cout << readServ(sock_fd) << std::endl;
    std::string pass = "1234";
    sprintf(str,"PASS %s\r\n", pass.data());
    send(sock_fd, str, (int)strlen(str),0);
    std::cout << readServ(sock_fd) << std::endl;
    // authorization

    while(true) {
        std::cout << "Please, enter the command: ";
        fflush(stdout);
        std::string command;
        std::cin >> command;
        if(command == "listfiles") {
            // open data channel
            send(sock_fd,"PASV\r\n",strlen("PASV\r\n"),0);
            std::string data = readServ(sock_fd);
            std::cout << data << std::endl;
            std::string_view view(data);
            view.remove_prefix(view.find('(') + 1);
            view = view.substr(0, view.find(')'));
            std::string in_braces(view);
            int buffer[6];
            sscanf(in_braces.data(), "%d,%d,%d,%d,%d,%d",
                   &buffer[0], &buffer[1], &buffer[2], &buffer[3], &buffer[4], &buffer[5]);
            // open data channel

            sprintf(str,"MLSD\r\n");
            send(sock_fd, str, (int)strlen(str), 0);
            std::cout << readServ(sock_fd) << std::endl;
            int ds = open_tcp_connection(uri.host, buffer[4] * 256 + buffer[5]);
            std::cout << readServ(ds) << std::endl;
            closesocket(ds);
            std::cout << readServ(sock_fd) << std::endl;
        } else if(command == "loadfile") {
            // open data channel
            send(sock_fd,"PASV\r\n",strlen("PASV\r\n"),0);
            std::string data = readServ(sock_fd);
            std::cout << data << std::endl;
            std::string_view view(data);
            view.remove_prefix(view.find('(') + 1);
            view = view.substr(0, view.find(')'));
            std::string in_braces(view);
            int buffer[6];
            sscanf(in_braces.data(), "%d,%d,%d,%d,%d,%d",
                   &buffer[0], &buffer[1], &buffer[2], &buffer[3], &buffer[4], &buffer[5]);
            // open data channel

            std::string filename;
            std::cout << "Please, enter the filename: ";
            fflush(stdout);
            std::cin >> filename;
            sprintf(str,"RETR %s\r\n", filename.data());
            send(sock_fd, str, (int)strlen(str),0);
            std::cout << readServ(sock_fd) << std::endl;
            int ds = open_tcp_connection(uri.host, buffer[4] * 256 + buffer[5]);
            FILE *f = fopen(filename.data(), "wb");
            data = readServ(ds);
            fwrite(data.data(),1, data.size(), f);
            fclose(f);
            closesocket(ds);
            std::cout << readServ(sock_fd) << std::endl;
        } else if(command == "exit") {
            break;
        } else {
            std::cout << "Unknown command!" << std::endl;
        }
    }
#ifdef WIN32
    WSACleanup();
#endif
    return 0;
}