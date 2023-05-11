### Инструкция по запуску утилиты ping:
Утилита умеет работать на Linux и на Windows. Пример запуска на Linux:
```console
astronomax@debian:~/CompNet$ sudo sysctl -w net.ipv4.ping_group_range="0 0"
[sudo] пароль для astronomax: 
net.ipv4.ping_group_range = 0 0
astronomax@debian:~/CompNet$ cd lab10/ping/
astronomax@debian:~/CompNet/lab10/ping$ mkdir build
astronomax@debian:~/CompNet/lab10/ping$ cmake -B./build -H. -DCMAKE_BUILD_TYPE=Release
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
-- Build files have been written to: /home/astronomax/CompNet/lab10/ping/build
astronomax@debian:~/CompNet/lab10/ping$ cd build
astronomax@debian:~/CompNet/lab10/ping/build$ make ping
Scanning dependencies of target ping
[ 50%] Building CXX object CMakeFiles/ping.dir/main.cpp.o
[100%] Linking CXX executable ping
[100%] Built target ping
astronomax@debian:~/CompNet/lab10/ping/build$ sudo ./ping akamai.com

Pinging akamai.com [104.86.181.144]

response from: 104.86.181.144
  RTT: 69 ms
  Type: 0
  Code: 0
  Identifier: 1024
  Sequence: 1
  Data: test packet
response from: 104.86.181.144
  RTT: 65 ms
  Type: 0
  Code: 0
  Identifier: 1024
  Sequence: 2
  Data: test packet
response from: 104.86.181.144
  RTT: 59 ms
  Type: 0
  Code: 0
  Identifier: 1024
  Sequence: 3
  Data: test packet
response from: 104.86.181.144
  RTT: 69 ms
  Type: 0
  Code: 0
  Identifier: 1024
  Sequence: 4
  Data: test packet
response from: 104.86.181.144
  RTT: 58 ms
  Type: 0
  Code: 0
  Identifier: 1024
  Sequence: 5
  Data: test packet
^C
astronomax@debian:~/CompNet/lab10/ping/build$ 
```
**Аргументы утилиты**: `{url}`, где `url` - адрес целевого сервера.  
*Замечание (при запуске на linux)*:  
Во-первых, необходимо выполнить:
```console 
sudo sysctl -w net.ipv4.ping_group_range="0 0"
```
Во-вторых, запускать утилиту необходимо под рутом.  
Иначе Вы получите ошибку при создании сокета.
