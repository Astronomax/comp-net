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

int pasv(int sock_fd) {
    send(sock_fd,"PASV\r\n",(int)strlen("PASV\r\n"),0);
    std::string data = readServ(sock_fd);
    std::cout << data << std::endl;
    std::string_view view(data);
    view.remove_prefix(view.find('(') + 1);
    view = view.substr(0, view.find(')'));
    std::string in_braces(view);
    int buffer[6];
    sscanf(in_braces.data(), "%d,%d,%d,%d,%d,%d",
           &buffer[0], &buffer[1], &buffer[2], &buffer[3], &buffer[4], &buffer[5]);
    return buffer[4] * 256 + buffer[5];
}

void sock_write(int sock_fd, const char *message) {
#ifdef WIN32
    send(sock_fd, message, (int)strlen(message), 0);
#else
    write(sock_fd, message, (int)strlen(message), 0);
#endif
}

void sock_fwrite(int sock_fd, const char *format, const char *args...) {
    int len = snprintf(nullptr, 0, format, args);
    std::string message(len, 0);
    sprintf(message.data(), format, args);
#ifdef WIN32
    send(sock_fd, message.data(), (int)message.size(), 0);
#else
    write(sock_fd, message.data(), message.size(), 0);
#endif
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

        if(command == "CDUP") {
            sock_write(sock_fd, "CDUP\r\n");
            std::cout << readServ(sock_fd) << std::endl;
        } else if(command == "CWD") {
            std::string dir;
            std::cin >> dir;
            sock_fwrite(sock_fd, "CWD %s\r\n", dir.data());
            std::cout << readServ(sock_fd) << std::endl;
        } else if(command == "PWD") {
            sock_write(sock_fd, "PWD\r\n");
            std::cout << readServ(sock_fd) << std::endl;
        } else if(command == "RETR") {
            int data_port = pasv(sock_fd);
            std::string file;
            std::cin >> file;
            sock_fwrite(sock_fd, "RETR %s\r\n", file.data());
            std::cout << readServ(sock_fd) << std::endl;
            int ds = open_tcp_connection(uri.host, data_port);
            FILE *f = fopen(file.data(), "wb");
            std::string data = readServ(ds);
            fwrite(data.data(),1, data.size(), f);
            fclose(f);
            closesocket(ds);
            std::cout << readServ(sock_fd) << std::endl;
        } else if (command == "STOR") {
            /*int data_port = pasv(sock_fd);
            std::string file;
            std::cin >> file;
            sock_fwrite(sock_fd, "STOR %s\r\n", file.data());
            int ds = open_tcp_connection(uri.host, data_port);
            //send
            closesocket(ds);
            std::cout << readServ(sock_fd) << std::endl;*/
            // TODO
            std::cout << "Unknown command!" << std::endl; //temporary unsupported
        } else if(command == "DELE") {
            std::string file;
            std::cin >> file;
            sock_fwrite(sock_fd, "DELE %s\r\n", file.data());
            std::cout << readServ(sock_fd) << std::endl;
        } else if(command == "MDTM") {
            std::string file;
            std::cin >> file;
            sock_fwrite(sock_fd, "MDTM %s\r\n", file.data());
            std::cout << readServ(sock_fd) << std::endl;
        } else if(command == "SIZE") {
            std::string file;
            std::cin >> file;
            sock_fwrite(sock_fd, "SIZE %s\r\n", file.data());
            std::cout << readServ(sock_fd) << std::endl;
        } else if(command == "SYST") {
            sock_write(sock_fd, "SYST\r\n");
            std::cout << readServ(sock_fd) << std::endl;
        } else if (command == "MKD") {
            std::string dir;
            std::cin >> dir;
            sock_fwrite(sock_fd, "MKD %s\r\n", dir.data());
            std::cout << readServ(sock_fd) << std::endl;
        } else if(command == "RMD") {
            std::string dir;
            std::cin >> dir;
            sock_fwrite(sock_fd, "RMD %s\r\n", dir.data());
            std::cout << readServ(sock_fd) << std::endl;
        } else if(command == "MLSD") {
            int data_port = pasv(sock_fd);
            sock_write(sock_fd, "MLSD\r\n");
            std::cout << readServ(sock_fd) << std::endl;
            int ds = open_tcp_connection(uri.host, data_port);
            std::cout << readServ(ds) << std::endl;
            closesocket(ds);
            std::cout << readServ(sock_fd) << std::endl;
        } else if(command == "NLST") {
            int data_port = pasv(sock_fd);
            sock_write(sock_fd, "NLST\r\n");
            std::cout << readServ(sock_fd) << std::endl;
            int ds = open_tcp_connection(uri.host, data_port);
            std::cout << readServ(ds) << std::endl;
            closesocket(ds);
            std::cout << readServ(sock_fd) << std::endl;
        } else if(command == "LIST") {
            int data_port = pasv(sock_fd);
            sock_write(sock_fd, "LIST\r\n");
            std::cout << readServ(sock_fd) << std::endl;
            int ds = open_tcp_connection(uri.host, data_port);
            std::cout << readServ(ds) << std::endl;
            closesocket(ds);
            std::cout << readServ(sock_fd) << std::endl;
        } else if (command == "NOOP") {
            sock_write(sock_fd, "NOOP\r\n");
            std::cout << readServ(sock_fd) << std::endl;
        } else if (command == "HELP") {
            sock_write(sock_fd, "HELP\r\n");
            std::cout << readServ(sock_fd) << std::endl;
        } else if(command == "QUIT") {
            sock_write(sock_fd, "QUIT\r\n");
            std::cout << readServ(sock_fd) << std::endl;
            break;
        } else {
            std::cout << "Unknown command!" << std::endl;
        }
    }
    closesocket(sock_fd);
#ifdef WIN32
    WSACleanup();
#endif
    return 0;
}