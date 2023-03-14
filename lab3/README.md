### Инструкция по запуску сервера:
```console
astronomax@astronomax:~/CompNet$ cd lab3
astronomax@astronomax:~/CompNet/lab3$ mkdir build
astronomax@astronomax:~/CompNet/lab3$ cmake -B./build -H. -DCMAKE_BUILD_TYPE=Release
-- The C compiler identification is GNU 10.2.1
-- The CXX compiler identification is GNU 10.2.1
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: /usr/bin/cc - skipped
-- Detecting C compile features
...
-- Configuring done
-- Generating done
-- Build files have been written to: /home/astronomax/CompNet/lab3/build
astronomax@astronomax:~/CompNet/lab3$ cd build
astronomax@astronomax:~/CompNet/lab3/build$ make server
[ 50%] Linking CXX executable server
[100%] Built target server
astronomax@astronomax:~/CompNet/lab3/build$ ./server 8000
Waiting for clients...

```
### Инструкция по запуску клиента:
```console
astronomax@astronomax:~/CompNet$ cd lab3
astronomax@astronomax:~/CompNet/lab3$ mkdir build
astronomax@astronomax:~/CompNet/lab3$ cmake -B./build -H. -DCMAKE_BUILD_TYPE=Release
-- The C compiler identification is GNU 10.2.1
-- The CXX compiler identification is GNU 10.2.1
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: /usr/bin/cc - skipped
-- Detecting C compile features
...
-- Configuring done
-- Generating done
-- Build files have been written to: /home/astronomax/CompNet/lab3/build
astronomax@astronomax:~/CompNet/lab3$ cd build
astronomax@astronomax:~/CompNet/lab3/build$ make client
[ 50%] Linking CXX executable server
[100%] Built target server
astronomax@astronomax:~/CompNet/lab3/build$ ./client 127.0.0.1 8000 /server
30003
astronomax@astronomax:~/CompNet$ 
```
