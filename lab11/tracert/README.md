### Инструкция по запуску утилиты tracert:
Утилита умеет работать на Linux и на Windows. Пример запуска на Linux:
```console
astronomax@debian:~/CompNet$ sudo sysctl -w net.ipv4.ping_group_range="0 0"
[sudo] пароль для astronomax: 
net.ipv4.ping_group_range = 0 0
astronomax@debian:~/CompNet$ cd lab11/tracert/
astronomax@debian:~/CompNet/lab11/tracert$ mkdir build
astronomax@debian:~/CompNet/lab11/tracert$ cmake -B./build -H. -DCMAKE_BUILD_TYPE=Release
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
-- Build files have been written to: /home/astronomax/CompNet/lab11/tracert/build
astronomax@debian:~/CompNet/lab11/tracert$ cd build
astronomax@debian:~/CompNet/lab11/tracert/build$ make tracert
Scanning dependencies of target tracert
[ 50%] Building CXX object CMakeFiles/tracert.dir/main.cpp.o
[100%] Linking CXX executable tracert
[100%] Built target tracert
astronomax@debian:~/CompNet/lab11/tracert/build$ sudo ./tracert akamai.com
[sudo] пароль для astronomax: 

Tracing route to akamai.com [23.56.197.176]
over a maximum of 30 hops:

 1     1 ms     1 ms     1 ms [192.168.0.1]
 2     2 ms     2 ms     2 ms [81.89.176.1]
 3     2 ms     2 ms     2 ms [195.70.196.3]
 4     2 ms     2 ms     2 ms [81.211.104.177]
 5    14 ms    13 ms    16 ms [79.104.229.53]
 6    14 ms    13 ms    13 ms [185.70.202.160]
 7   216 ms   203 ms   204 ms [89.221.41.161]
 8   205 ms   206 ms   203 ms [89.221.41.247]
 9   206 ms   204 ms   205 ms [23.207.239.129]
10     *        *        *     The waiting interval for the request has been exceeded.
11     *        *        *     The waiting interval for the request has been exceeded.
12     *        *        *     The waiting interval for the request has been exceeded.
13   157 ms   157 ms   157 ms [23.56.197.176]

Trace complete.
```
**Аргументы утилиты**: `{url}`, где `url` - адрес целевого сервера.  
*Замечание (при запуске на linux)*:  
Во-первых, необходимо выполнить:
```console 
sudo sysctl -w net.ipv4.ping_group_range="0 0"
```
Во-вторых, запускать утилиту необходимо под рутом.  
Иначе Вы получите ошибку при создании сокета.
