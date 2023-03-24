### Инструкция по запуску прокси-сервера:
```console
astronomax@astronomax:~/CompNet$ cd lab4
astronomax@astronomax:~/CompNet/lab4$ mkdir build
astronomax@astronomax:~/CompNet/lab4$ cmake -B./build -H. -DCMAKE_BUILD_TYPE=Release
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
...
-- Configuring done
-- Generating done
-- Build files have been written to: /home/astronomax/CompNet/lab4/build
astronomax@astronomax:~/CompNet/lab4$ cd build
astronomax@astronomax:~/CompNet/lab4/build$ make server
Scanning dependencies of target server
[ 25%] Building CXX object CMakeFiles/server.dir/src/server/main.cpp.o
[ 50%] Building CXX object CMakeFiles/server.dir/open_tcp_connection/open_tcp_connection.cpp.o
[ 75%] Building CXX object CMakeFiles/server.dir/my_http/my_http.cpp.o
[100%] Linking CXX executable server
[100%] Built target server
astronomax@astronomax:~/CompNet/lab4/build$ ./server 8000
Waiting for clients...

```
**Аргументы сервера**: `{port}`  
Список заблокированных хостов должен находиться в файле `blacklist` рядом с исполняемым файлом.  
В процессе работы сервер будет писать лог в файл `log.txt` в формате `{host}: {status code}`.  
Также в процессе работы сервер будет создавать возле себя файлы с названием `cache_{request hash}`, в которых будет кешировать ответы на запросы, содержащие поле `Last-Modified`.   
Сервер способен принимать запросы любого типа (в т.ч. `POST`).

### Инструкция по запуску клиента:
```console
astronomax@astronomax:~/CompNet$ cd lab4
astronomax@astronomax:~/CompNet/lab4$ mkdir build
astronomax@astronomax:~/CompNet/lab4$ cmake -B./build -H. -DCMAKE_BUILD_TYPE=Release
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
...
-- Configuring done
-- Generating done
-- Build files have been written to: /home/astronomax/CompNet/lab4/build
astronomax@astronomax:~/CompNet/lab4/build$ make client
Scanning dependencies of target client
[ 25%] Building CXX object CMakeFiles/client.dir/src/client/main.cpp.o
[ 50%] Building CXX object CMakeFiles/client.dir/open_tcp_connection/open_tcp_connection.cpp.o
[ 75%] Building CXX object CMakeFiles/client.dir/my_http/my_http.cpp.o
[100%] Linking CXX executable client
[100%] Built target client
astronomax@astronomax:~/CompNet/lab4/build$ ./client 127.0.0.1:8000/gaia.cs.umass.edu/wireshark-labs/HTTP-wireshark-file3.html
astronomax@astronomax:~/CompNet$ 
```
**Аргументы клиента**: `{uri}`  
Клиент отправляет запрос на указанный uri в формате `GET {uri.path}{uri.query} HTTP/1.0\r\nHost: {uri.host}\r\n\r\n`.  
Ответ сервера записывается в файл `response`, находящийся рядом с исполняемым файлом клиента.
