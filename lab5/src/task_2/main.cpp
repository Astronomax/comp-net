#include <cstdio>
#include <unistd.h>
#include <string>
#include "../../open_tcp_connection/open_tcp_connection.hpp"
#include <fstream>
#include <cstring>
#include "../../data_batch/data_batch.hpp"
#include <fcntl.h>
#include <iostream>
#include <array>

#define SMTP_SERVER  "mail.student.spbu.ru"

class base_64_encoder {
    constexpr static const char encode_table[64] = {
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
            'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
            'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g',
            'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
            's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2',
            '3', '4', '5', '6', '7', '8', '9', '+', '/'};

    static std::array<char, 4> encode_triplet(std::int32_t a, std::int32_t b, std::int32_t c) {
        std::uint32_t const concat_bits = (a << 16) | (b << 8) | c;
        auto const b64_char1 = encode_table[(concat_bits >> 18) & 0b0011'1111];
        auto const b64_char2 = encode_table[(concat_bits >> 12) & 0b0011'1111];
        auto const b64_char3 = encode_table[(concat_bits >> 6) & 0b0011'1111];
        auto const b64_char4 = encode_table[concat_bits & 0b0011'1111];
        return {b64_char1, b64_char2, b64_char3, b64_char4};
    }

public:
    static std::string encode(const std::string &data) {
        std::string code;
        for (int i = 0; i < data.size(); i += 3) {
            std::string triplet(3, 0);
            for(int j = 0; j < 3 && i + j < data.size(); j++) {
                triplet[j] = data[i + j];
            }
            auto triplet_code = encode_triplet(triplet[0], triplet[1], triplet[2]);
            code += std::string(triplet_code.data(), triplet_code.size());
        }
        return code;
    }
};

static const std::string payload_text_format =
        "To: <%s>\r\n"
        "From: Anton Kuznets <%s>\r\n"
        "Sender: Anton Kuznets <%s>\r\n"
        "Message-ID: <dcd7cb36-11db-487a-9f3a-e652a9458efd@"
        "rfcpedant.example.org>\r\n"
        "Subject: SMTP example message\r\n"
        "\r\n"
        "The body of the message starts here.\r\n"
        "\r\n"
        "It could be a lot of lines, could be MIME encoded, whatever.\r\n"
        "Check RFC5322.\r\n";

void read_config(std::string &username, std::string &password) {
    std::ifstream f("./config.txt");
    f >> username >> password;
    f.close();
}

void write(int sock_fd, const char *message) {
    write(sock_fd, message, strlen(message));
}

void fwrite(int sock_fd, const char *format, const char *args...) {
    int len = snprintf(nullptr, 0, format, args);
    std::string message(len, 0);
    sprintf(message.data(), format, args);
    write(sock_fd, message.data(), message.size());
}

std::string read_data(int sock_fd) {
    std::string data;
    data_batch batch{};
    fcntl(sock_fd, F_SETFL, (fcntl(sock_fd, F_GETFL, 0) & ~O_NONBLOCK));
    batch.data_len = read(sock_fd, batch.data, data_batch::BUFFER_SIZE);
    fcntl(sock_fd, F_SETFL, (fcntl(sock_fd, F_GETFL, 0) | O_NONBLOCK));
    while(batch.data_len > 0) {
        data += std::string(batch.data, batch.data_len);
        batch.data_len = read(sock_fd, batch.data, data_batch::BUFFER_SIZE);
    }
    return data;
}

int parse_code(const std::string &data) {
    return std::stoi(data.substr(0, data.find(' ')));
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Too few arguments. Expected {TO_ADDR}`\n");
        return 1;
    }

    std::string rcpt_to(argv[1]);
    std::string username, password;
    read_config(username, password);

    const int sock_fd = open_tcp_connection(SMTP_SERVER, 25);

    if (parse_code(read_data(sock_fd)) != 220) {
        fprintf(stderr, "Expected code 220 after connection\n");
        return 1;
    }
    fwrite(sock_fd, "HELO %s\r\n", username.data());
    if (parse_code(read_data(sock_fd)) != 250) {
        fprintf(stderr, "Expected code 250 after `HELO`\n");
        return 1;
    }
    write(sock_fd, "AUTH LOGIN\r\n");
    if (parse_code(read_data(sock_fd)) != 334) {
        fprintf(stderr, "Expected code 334 after `AUTH LOGIN`\n");
        return 1;
    }
    auto encoded_username = base_64_encoder::encode(username);
    fwrite(sock_fd, "%s\r\n", encoded_username.data());
    if (parse_code(read_data(sock_fd)) != 334) {
        fprintf(stderr, "Expected code 334 after `username`\n");
        return 1;
    }
    auto encoded_password = base_64_encoder::encode(password);
    fwrite(sock_fd, "%s\r\n", encoded_password.data());
    if (parse_code(read_data(sock_fd)) != 235) {
        fprintf(stderr, "Expected code 235 after `password`\n");
        return 1;
    }
    fwrite(sock_fd, "MAIL FROM: %s\r\n", username.data());
    if (parse_code(read_data(sock_fd)) != 250) {
        fprintf(stderr, "Expected code 250 after `MAIL FROM`\n");
        return 1;
    }
    fwrite(sock_fd, "RCPT TO: %s\r\n", rcpt_to.data());
    if (parse_code(read_data(sock_fd)) != 250) {
        fprintf(stderr, "Expected code 250 after `RCPT TO`\n");
        return 1;
    }
    write(sock_fd, "DATA\r\n");
    if (parse_code(read_data(sock_fd)) != 354) {
        fprintf(stderr, "Expected code 354 after `DATA`\n");
        return 1;
    }
    fwrite(sock_fd, payload_text_format.data(), username.data(), username.data(), rcpt_to.data());
    write(sock_fd, ".\r\n");
    if (parse_code(read_data(sock_fd)) != 250) {
        fprintf(stderr, "Expected code 250 after `.`\n");
        return 1;
    }
    write(sock_fd, "QUIT\r\n");
    std::cout << read_data(sock_fd) << std::endl;

    close(sock_fd);
    return 0;
}