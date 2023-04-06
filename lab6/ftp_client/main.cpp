#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include "open_tcp_connection/open_tcp_connection.hpp"
#include "uri/uri.hpp"

int readServ(int s) {
    int rc;
    fd_set fdr;
    FD_ZERO(&fdr);
    FD_SET(s,&fdr);
    timeval timeout{};
    timeout.tv_sec = 1;   ///зададим  структуру времени со значением 1 сек
    timeout.tv_usec = 0;
    do {
        char buff[512] ={' '};
        recv(s,&buff,512,0);  ///получаем данные из потока
        std::cout << buff;
        rc = select(s+1,&fdr,nullptr,nullptr,&timeout);    ///ждём данные для чтения в потоке 1 сек. подробнее о select
    } while(rc);     ///проверяем результат
    return 2;
}

int init_data(std::string addr, int s) {
    send(s,"PASV\r\n",strlen("PASV\r\n"),0);
    char buff[128];
    recv(s,buff,128,0);
    std::cout << buff; ////выводим на экран полученную от сервера строку
    int a,b;
    char *tmp_char;
    tmp_char = strtok(buff,"(");
    tmp_char = strtok(nullptr,"(");
    tmp_char = strtok(tmp_char, ")");
    int c,d,e,f;
    sscanf(tmp_char, "%d,%d,%d,%d,%d,%d",&c,&d,&e,&f,&a,&b);
    int len;
    sockaddr_in address{};
    int result;
    int port = a*256 + b;
    int ds = socket(AF_INET, SOCK_STREAM,0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(addr.data());    ///addr - у меня глобальная переменная с адресом сервера
    address.sin_port = htons(port);
    len = sizeof(address);
    result = connect(ds, (sockaddr *)&address, len);
    if (result == -1) {
        perror("oops: client");
        return -1;
    }
    return 0;
}

int login(int s) {
    std::cout << "Введите имя: "; char name[64]; std::cin >> name;
    char str[512];
    sprintf(str,"USER %s\r\n",name);
    send(s,str,strlen(str),0);
    readServ(s);
    std::cout << "Введите пароль: "; char pass[64]; std::cin >> pass;
    sprintf(str,"PASS %s\r\n",pass);
    send(s,str,strlen(str),0);
    readServ(s);
    return 0;
}


int get(char *file, int s, int ds) {
    char str[512];
    sprintf(str,"RETR %s\r\n",file);
    send(s,str,strlen(str),0);

    //получение размера файла
    char size[512];
    recv(s,size,512,0);
    std::cout << size;

    char *tmp_size;
    tmp_size = strtok(size,"(");
    tmp_size = strtok(nullptr,"(");
    tmp_size = strtok(tmp_size, ")");
    tmp_size = strtok(tmp_size, " ");

    int file_size;
    sscanf(tmp_size,"%d",&file_size);
    FILE *f;
    f = fopen(file, "wb");   ///важно чтобы файл писался в бинарном режиме
    int read = 0;  ///изначально прочитано 0 байт
    do {
        char buff[2048];   ////буфе для данных
        int readed = recv(ds,buff,sizeof(buff),0);   ///считываем данные с сервера. из сокета данных
        fwrite(buff,1,readed,f);   ///записываем считанные данные в файл
        read += readed;  ///увеличиваем количество скачанных данных
    } while (read < file_size);
    fclose(f);
    std::cout << "Готово. Ожидание ответа сервера...\n";
    return 0;
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

    int sock_fd = open_tcp_connection(uri.host, (uri.port.empty() ? 21 : stoi(uri.port)));
    readServ(sock_fd);
    close(sock_fd);
    return 0;
}