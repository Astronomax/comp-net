### Инструкция по запуску ftp-клиента:
Клиент умеет работать на Linux и на Windows. Пример запуска на Linux:
```console
astronomax@astronomax:~/CompNet$ cd lab6/ftp_client
astronomax@astronomax:~/CompNet/lab6/ftp_client$ mkdir build
astronomax@debian:~/CompNet/lab6/ftp_client$ cmake -B./build -H. -DCMAKE_BUILD_TYPE=Release
-- The C compiler identification is GNU 10.2.1
-- The CXX compiler identification is GNU 10.2.1
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: /usr/bin/cc - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/c++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Configuring done
-- Generating done
-- Build files have been written to: /home/astronomax/CompNet/lab6/ftp_client/build
astronomax@debian:~/CompNet/lab6/ftp_client$ cd build
astronomax@debian:~/CompNet/lab6/ftp_client/build$ make ftp_client
Scanning dependencies of target ftp_client
[ 25%] Building CXX object CMakeFiles/ftp_client.dir/main.cpp.o
[ 50%] Building CXX object CMakeFiles/ftp_client.dir/open_tcp_connection/open_tcp_connection.cpp.o
[ 75%] Building CXX object CMakeFiles/ftp_client.dir/uri/uri.cpp.o
[100%] Linking CXX executable ftp_client
[100%] Built target ftp_client
astronomax@debian:~/CompNet/lab6/ftp_client/build$ echo "TestUser 1234" > config
astronomax@debian:~/CompNet/lab6/ftp_client/build$ ./ftp_client ftp://127.0.0.1:21 ./config
220 (vsFTPd 3.0.3)

331 Please specify the password.

230 Login successful.

Please, enter the command:

```
**Аргументы клиента**: `{uri} {config_path}`, где `uri` - адрес управляющего соединения ftp-сервера, `config_path` - путь к файлу с логином и паролем для авторизации на сервере. 
В `uri` обязательно нужно указывать указывать протокол (префикс `ftp://`).
config-файл должен иметь содержимое вида `{username} {password}` (логин и пароль, разделенные пробелом).
Пример валидных аргументов клиента `ftp://127.0.0.1:21 ./config`
