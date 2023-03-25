#include <cstdio>
#include <cstring>
#include <curl/curl.h>
#include <algorithm>
#include <string>
#include <fstream>

#define FROM_ADDR    "<st086131@student.spbu.ru>"

static const char *payload_text_format =
        "To: <%s>\r\n"
        "From: Anton Kuznets " FROM_ADDR "\r\n"
        "Sender: Anton Kuznets " FROM_ADDR "\r\n"
        "Message-ID: <dcd7cb36-11db-487a-9f3a-e652a9458efd@"
        "rfcpedant.example.org>\r\n"
        "Subject: SMTP example message\r\n"
        "\r\n"
        "The body of the message starts here.\r\n"
        "\r\n"
        "It could be a lot of lines, could be MIME encoded, whatever.\r\n"
        "Check RFC5322.\r\n";

struct upload_args {
    const char *payload_text;
    size_t bytes_read;
};

static size_t payload_source(char *ptr, size_t size, size_t nmemb, void *userp) {
    auto upload_ctx = reinterpret_cast<upload_args *>(userp);
    size_t room = size * nmemb;
    if(room == 0) {
        return 0;
    }

    const char *data = upload_ctx->payload_text + upload_ctx->bytes_read;

    if(data) {
        size_t len = strlen(data);
        len = std::min(len, room);
        memcpy(ptr, data, len);
        upload_ctx->bytes_read += len;
        return len;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Too few arguments. Expected {TO_ADDR}`\n");
        return 1;
    }

    CURL *curl;
    CURLcode res = CURLE_OK;
    struct curl_slist *recipients = NULL;

    std::string payload_text(strlen(payload_text_format) + strlen(argv[1]) - 2, 0);
    sprintf(payload_text.data(), payload_text_format, argv[1]);

    upload_args upload_ctx = { payload_text.c_str(), 0 };

    std::string username;
    std::string password;
    std::ifstream f("./config.txt");
    f >> username >> password;
    f.close();

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "smtp://smtp.gmail.com");
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());
        curl_easy_setopt(curl, CURLOPT_LOGIN_OPTIONS, "AUTH=PLAIN");
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, FROM_ADDR);
        recipients = curl_slist_append(recipients, argv[1]);
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
        curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        res = curl_easy_perform(curl);

        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));

        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);
    }
    return (int)res;
}