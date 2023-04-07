#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#endif

#include <cstdio>
#include <iostream>
#include "uri/uri.hpp"
#include <fstream>
#include <cstdarg>

#include "data_batch/data_batch.hpp"
#include "open_tcp_connection/open_tcp_connection.hpp"

std::string read_data(int sock_fd) {
    fd_set fdr;
    FD_ZERO(&fdr);
    FD_SET(sock_fd,&fdr);
    timeval timeout{};
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    std::string res;
    data_batch batch{};
    int rc;
    do {
        std::fill(batch.data, batch.data + data_batch::BUFFER_SIZE, 0);
        batch.data_len = recv(sock_fd, batch.data, data_batch::BUFFER_SIZE, 0);
        if(batch.data_len <= 0) break;
        res += std::string(batch.data, batch.data_len);
        rc = select(0, &fdr,nullptr,nullptr, &timeout);
    } while(rc);
    return res;
}

int pasv(int sock_fd) {
    send(sock_fd,"PASV\r\n",(int)strlen("PASV\r\n"),0);
    std::string data = read_data(sock_fd);
    std::cout << data << std::endl;
    std::string_view view(data);
    view.remove_prefix(view.find('(') + 1);
    view = view.substr(0, view.find(')'));
    std::string in_braces(view);
    char *pEnd = in_braces.data();
    strtol(pEnd, &pEnd, 10);
    for(int i = 0; i < 3; i++) {
        strtol(pEnd + 1, &pEnd, 10);
    }
    int a = strtol(pEnd + 1, &pEnd, 10);
    int b = strtol(pEnd + 1, &pEnd,  10);
    return a * 256 + b;
}

void sock_write(int sock_fd, const char *message) {
    send(sock_fd, message, (int)strlen(message), 0);
}

void sock_write(int sock_fd, const char *message, int len) {
    send(sock_fd, message, len, 0);
}

void sock_fwrite(int sock_fd, const char *format, ...) {
    va_list argv;
    va_start(argv, format);
    int len = vsnprintf(nullptr, 0, format, argv);
    std::string message(len, 0);
    vsprintf(message.data(), format, argv);
    va_end(argv);
    send(sock_fd, message.data(), (int)message.size(), 0);
}

void sock_close(int sock_fd) {
#ifdef WIN32
    closesocket(sock_fd);
#else
    close(sock_fd);
#endif
}

void auth(int sock_fd, const std::string& config_path) {
    std::ifstream f(config_path);
    std::string user, pass;
    f >> user >> pass;
    f.close();
    sock_fwrite(sock_fd, "USER %s\r\n", user.data());
    std::cout << read_data(sock_fd) << std::endl;
    sock_fwrite(sock_fd, "PASS %s\r\n", pass.data());
    std::cout << read_data(sock_fd) << std::endl;
}

int ftp_parse_status_code(const std::string &response) {
    std::string_view view(response);
    view = view.substr(0, view.find(' '));
    return std::stoi(std::string(view));
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Too few arguments. Expected {uri} {config_path}`\n");
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

    int port = (uri.port.empty() ? 21 : stoi(uri.port));
    int sock_fd = open_tcp_connection(uri.host, port);
    std::cout << read_data(sock_fd) << std::endl;
    auth(sock_fd, argv[2]);

    while(true) {
        std::cout << "Please, enter the command:" << std::endl;
        std::string command;
        std::cin >> command;

        if(command == "RNFR") {
            std::string file; std::cin >> file;
            sock_fwrite(sock_fd, "RNFR %s\r\n", file.data());
            std::cout << read_data(sock_fd) << std::endl;
        } else if(command == "RNTO") {
            std::string file;
            std::cin >> file;
            sock_fwrite(sock_fd, "RNTO %s\r\n", file.data());
            std::cout << read_data(sock_fd) << std::endl;
        } else if(command == "NLST") {
            std::string fnop;
            std::cin >> fnop;
            int data_port = pasv(sock_fd);
            sock_fwrite(sock_fd, "%s %s\r\n", command.data(), fnop.data());
            std::string response = read_data(sock_fd);
            std::cout << response << std::endl;
            if (ftp_parse_status_code(response) == 150) {
                int ds = open_tcp_connection(uri.host, data_port);
                std::cout << read_data(ds) << std::endl;
                sock_close(ds);
                std::cout << read_data(sock_fd) << std::endl;
            }
        } else if(command == "MLST") {
            std::string fnop;
            std::cin >> fnop;
            sock_fwrite(sock_fd, "%s %s\r\n", command.data(), fnop.data());
            std::cout << read_data(sock_fd) << std::endl;
        } else if(command == "CLNT") {
            sock_write(sock_fd, "CLNT\r\n");
            std::cout << read_data(sock_fd) << std::endl;
        } else if(command == "FEAT") {
            sock_write(sock_fd, "FEAT\r\n");
            std::cout << read_data(sock_fd) << std::endl;
        } else if(command == "STAT") {
            sock_write(sock_fd, "STAT\r\n");
            std::cout << read_data(sock_fd) << std::endl;
        } else if(command == "CDUP") {
            sock_write(sock_fd, "CDUP\r\n");
            std::cout << read_data(sock_fd) << std::endl;
        } else if(command == "CWD" || command == "XCWD") {
            std::string dir;
            std::cin >> dir;
            sock_fwrite(sock_fd, "%s %s\r\n", command.data(), dir.data());
            std::cout << read_data(sock_fd) << std::endl;
        } else if(command == "PWD" || command == "XPWD") {
            sock_write(sock_fd, "PWD\r\n");
            std::cout << read_data(sock_fd) << std::endl;
        } else if(command == "RETR") {
            int data_port = pasv(sock_fd);
            std::string src_file, dst_file;
            std::cin >> src_file >> dst_file;
            sock_fwrite(sock_fd, "RETR %s\r\n", src_file.data());
            std::string response = read_data(sock_fd);
            std::cout << response << std::endl;
            if (ftp_parse_status_code(response) == 150) {
                int ds = open_tcp_connection(uri.host, data_port);
                std::ofstream file(dst_file, std::ostream::binary);
                std::string data = read_data(ds);
                file.write(data.data(), (int)data.size());
                file.close();
                sock_close(ds);
                std::cout << read_data(sock_fd) << std::endl;
            }
        } else if (command == "STOR") {
            int data_port = pasv(sock_fd);
            std::string dst_file, src_file;
            std::cin >> dst_file >> src_file;
            sock_fwrite(sock_fd, "STOR %s\r\n", dst_file.data());
            std::string response = read_data(sock_fd);
            std::cout << response << std::endl;
            if (ftp_parse_status_code(response) == 150) {
                int ds = open_tcp_connection(uri.host, data_port);
                std::ifstream file(src_file);
                file.seekg(0, std::ios::end);
                std::streamsize file_size = file.tellg();
                std::string file_bytes(file_size, 0);
                file.seekg(0);
                file.read(file_bytes.data(), file_size);
                file.close();
                sock_write(ds, file_bytes.data(), (int)file_size);
                sock_close(ds);
                std::cout << read_data(sock_fd) << std::endl;
            }
        } else if(command == "DELE") {
            std::string file;
            std::cin >> file;
            sock_fwrite(sock_fd, "DELE %s\r\n", file.data());
            std::cout << read_data(sock_fd) << std::endl;
        } else if(command == "MDTM") {
            std::string file;
            std::cin >> file;
            sock_fwrite(sock_fd, "MDTM %s\r\n", file.data());
            std::cout << read_data(sock_fd) << std::endl;
        } else if(command == "SIZE") {
            std::string file;
            std::cin >> file;
            sock_fwrite(sock_fd, "SIZE %s\r\n", file.data());
            std::cout << read_data(sock_fd) << std::endl;
        } else if(command == "SYST") {
            sock_write(sock_fd, "SYST\r\n");
            std::cout << read_data(sock_fd) << std::endl;
        } else if (command == "MKD" || command == "XMKD") {
            std::string dir;
            std::cin >> dir;
            sock_fwrite(sock_fd, "%s %s\r\n", command.data(), dir.data());
            std::cout << read_data(sock_fd) << std::endl;
        } else if(command == "RMD" || command == "XRMD") {
            std::string dir;
            std::cin >> dir;
            sock_fwrite(sock_fd, "%s %s\r\n", command.data(), dir.data());
            std::cout << read_data(sock_fd) << std::endl;
        } else if(command == "MLSD" || command == "LIST") {
            int data_port = pasv(sock_fd);
            sock_fwrite(sock_fd, "%s\r\n", command.data());
            std::string response = read_data(sock_fd);
            std::cout << response << std::endl;
            if (ftp_parse_status_code(response) == 150) {
                int ds = open_tcp_connection(uri.host, data_port);
                std::cout << read_data(ds) << std::endl;
                sock_close(ds);
                std::cout << read_data(sock_fd) << std::endl;
            }
        } else if (command == "NOOP" || command == "NOP") {
            sock_fwrite(sock_fd, "%s\r\n", command.data());
            std::cout << read_data(sock_fd) << std::endl;
        } else if (command == "HELP") {
            sock_write(sock_fd, "HELP\r\n");
            std::cout << read_data(sock_fd) << std::endl;
        } else if(command == "QUIT") {
            sock_write(sock_fd, "QUIT\r\n");
            std::cout << read_data(sock_fd) << std::endl;
            break;
        } else {
            std::cout << "Unknown command!" << std::endl;
        }
    }
    sock_close(sock_fd);
#ifdef WIN32
    WSACleanup();
#endif
    return 0;
}