#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <cstdlib>

#include "uri/uri.hpp"
#include "data_batch/data_batch.hpp"
#include "open_tcp_connection/open_tcp_connection.hpp"

void sock_write(int sock_fd, const char *message) {
    send(sock_fd, message, (int)strlen(message), 0);
}

void sock_write(int sock_fd, const char *message, int len) {
    send(sock_fd, message, len, 0);
}

void sock_fwrite(int sock_fd, const char *format, ...) {
    va_list argv;
    va_start(argv, format);
    va_end(argv);

    va_list a;
    va_copy(a, argv);
    int len = vsnprintf(nullptr, 0, format, a);
    std::string message(len, 0);
    va_list b;
    va_copy(b, argv);
    vsprintf(message.data(), format, b);
    send(sock_fd, message.data(), (int)message.size(), 0);
}

void sock_close(int sock_fd) {
#ifdef WIN32
    closesocket(sock_fd);
#else
    close(sock_fd);
#endif
}

class control_channel {
    std::string m_res;
    int m_sock_fd;
public:
    explicit control_channel(int sock_fd) : m_sock_fd(sock_fd) {}

    std::string read_control_data() {
        data_batch batch{};
        while(true) {
            int ind = m_res.find("\r\n");
            if (ind != -1) {
                std::string data = m_res.substr(0, ind + 2);
                m_res = m_res.substr(ind + 2, m_res.size() - ind - 2);
                return data;
            }
            batch.data_len = recv(m_sock_fd, batch.data, data_batch::BUFFER_SIZE, 0);
            m_res += std::string(batch.data, batch.data_len);
        }
    }

    void write(const char *message) {
        sock_write(m_sock_fd, message);
    }

    void write(const char *message, int len) {
        sock_write(m_sock_fd, message, len);
    }

    void fwrite(const char *format, ...) {
        va_list argv;
        va_start(argv, format);
        va_end(argv);

        va_list a;
        va_copy(a, argv);
        int len = vsnprintf(nullptr, 0, format, a);
        std::string message(len, 0);
        va_list b;
        va_copy(b, argv);
        vsprintf(message.data(), format, b);
        send(m_sock_fd, message.data(), (int)message.size(), 0);
    }
};

std::string read_data(int sock_fd) {
    std::string res;
    data_batch batch{};

    fd_set fdr;
    FD_ZERO(&fdr);
    FD_SET(sock_fd, &fdr);
    timeval timeout{};
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    int rc;
    do {
        batch.data_len = recv(sock_fd, batch.data, data_batch::BUFFER_SIZE, 0);
        if(batch.data_len <= 0) break;
        res += std::string(batch.data, batch.data_len);
        rc = select(0, &fdr, nullptr, nullptr, &timeout);
    } while(rc);
    return res;
}

long pasv(control_channel &control) {
    control.write("PASV\r\n");
    std::string data = control.read_control_data();
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
    long a = strtol(pEnd + 1, &pEnd, 10);
    long b = strtol(pEnd + 1, &pEnd, 10);
    return a * 256 + b;
}

void auth(control_channel &control, const std::string& config_path) {
    std::ifstream f(config_path);
    std::string user, pass;
    f >> user >> pass;
    f.close();
    control.fwrite("USER %s\r\n", user.data());
    std::cout << control.read_control_data() << std::endl;
    control.fwrite("PASS %s\r\n", pass.data());
    std::cout << control.read_control_data() << std::endl;
}

int ftp_parse_status_code(const std::string &response) {
    std::string_view view(response);
    view = view.substr(0, 3);
    try {
        return std::stoi(std::string(view));
    } catch (...) {
        return -1;
    }
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

    control_channel control(sock_fd);
    std::cout << control.read_control_data() << std::endl;
    auth(control, argv[2]);

    while(true) {
        std::cout << "Please, enter the command:" << std::endl;
        std::string command;
        std::cin >> command;

        if(command == "RNFR") {
            std::string file; std::cin >> file;
            control.fwrite("RNFR %s\r\n", file.data());
            std::cout << control.read_control_data() << std::endl;
        } else if(command == "RNTO") {
            std::string file;
            std::cin >> file;
            control.fwrite("RNTO %s\r\n", file.data());
            std::cout << control.read_control_data() << std::endl;
        } else if(command == "NLST") {
            std::string fnop;
            std::cin >> fnop;
            long data_port = pasv(control);
            control.fwrite("%s %s\r\n", command.data(), fnop.data());

            int ds = open_tcp_connection(uri.host, data_port);
            std::string response = control.read_control_data();
            std::cout << response << std::endl;

            if (ftp_parse_status_code(response) == 150) {
                std::cout << read_data(ds) << std::endl;
            }
            sock_close(ds);
        } else if(command == "MLST") {
            std::string fnop;
            std::cin >> fnop;
            control.fwrite("%s %s\r\n", command.data(), fnop.data());
            std::cout << control.read_control_data() << std::endl;
        } else if(command == "CLNT") {
            control.fwrite("CLNT\r\n");
            std::cout << control.read_control_data() << std::endl;
        } else if(command == "FEAT") {
            control.fwrite("FEAT\r\n");
            std::cout << control.read_control_data() << std::endl;
        } else if(command == "STAT") {
            control.fwrite("STAT\r\n");
            std::cout << control.read_control_data() << std::endl;
        } else if(command == "CDUP") {
            control.fwrite("CDUP\r\n");
            std::cout << control.read_control_data() << std::endl;
        } else if(command == "CWD" || command == "XCWD") {
            std::string dir;
            std::cin >> dir;
            control.fwrite("%s %s\r\n", command.data(), dir.data());
            std::cout << control.read_control_data() << std::endl;
        } else if(command == "PWD" || command == "XPWD") {
            control.fwrite("PWD\r\n");
            std::cout << control.read_control_data() << std::endl;
        } else if(command == "RETR") {
            long data_port = pasv(control);
            std::string src_file, dst_file;
            std::cin >> src_file >> dst_file;
            control.fwrite("RETR %s\r\n", src_file.data());

            int ds = open_tcp_connection(uri.host, data_port);
            std::string response = control.read_control_data();
            std::cout << response << std::endl;

            if (ftp_parse_status_code(response) == 150) {
                std::ofstream file(dst_file, std::ostream::binary);
                std::string data = read_data(ds);
                file.write(data.data(), (int)data.size());
                file.close();
            }
            sock_close(ds);
        } else if (command == "STOR") {
            long data_port = pasv(control);
            std::string dst_file, src_file;
            std::cin >> dst_file >> src_file;
            control.fwrite("STOR %s\r\n", dst_file.data());

            int ds = open_tcp_connection(uri.host, data_port);
            std::string response = control.read_control_data();
            std::cout << response << std::endl;

            if (ftp_parse_status_code(response) == 150) {
                std::ifstream file(src_file);
                file.seekg(0, std::ios::end);
                std::streamsize file_size = file.tellg();
                std::string file_bytes(file_size, 0);
                file.seekg(0);
                file.read(file_bytes.data(), file_size);
                file.close();
                sock_write(ds, file_bytes.data(), (int)file_size);
                std::cout << control.read_control_data() << std::endl;
            }
            sock_close(ds);
        } else if(command == "DELE") {
            std::string file;
            std::cin >> file;
            control.fwrite("DELE %s\r\n", file.data());
            std::cout << control.read_control_data() << std::endl;
        } else if(command == "MDTM") {
            std::string file;
            std::cin >> file;
            control.fwrite("MDTM %s\r\n", file.data());
            std::cout << control.read_control_data() << std::endl;
        } else if(command == "SIZE") {
            std::string file;
            std::cin >> file;
            control.fwrite("SIZE %s\r\n", file.data());
            std::cout << control.read_control_data() << std::endl;
        } else if(command == "SYST") {
            control.fwrite("SYST\r\n");
            std::cout << control.read_control_data() << std::endl;
        } else if (command == "MKD" || command == "XMKD") {
            std::string dir;
            std::cin >> dir;
            control.fwrite("%s %s\r\n", command.data(), dir.data());
            std::cout << control.read_control_data() << std::endl;
        } else if(command == "RMD" || command == "XRMD") {
            std::string dir;
            std::cin >> dir;
            control.fwrite("%s %s\r\n", command.data(), dir.data());
            std::cout << control.read_control_data() << std::endl;
        } else if(command == "MLSD" || command == "LIST") {
            long data_port = pasv(control);
            control.fwrite("%s\r\n", command.data());

            int ds = open_tcp_connection(uri.host, data_port);
            std::string response = control.read_control_data();
            std::cout << response << std::endl;
            response = control.read_control_data();
            std::cout << response << std::endl;

            if (ftp_parse_status_code(response) == 150) {
                std::cout << read_data(ds) << std::endl;
            }
            sock_close(ds);
        } else if (command == "NOOP" || command == "NOP") {
            control.fwrite("%s\r\n", command.data());
            std::cout << control.read_control_data() << std::endl;
        } else if (command == "HELP") {
            control.fwrite("HELP\r\n");
            std::string response = control.read_control_data();
            std::cout << response;
            int code = ftp_parse_status_code(response);
            do {
                response = control.read_control_data();
                std::cout << response;
            }
            while(ftp_parse_status_code(response) != code);
            std::cout << std::endl;
        } else if(command == "QUIT") {
            control.fwrite("QUIT\r\n");
            std::cout << control.read_control_data() << std::endl;
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