1.
    1.  Мой браузер использует версию http 1.1, такая же работает и на сервере.
    2.  ru,en;q=0.9,en-GB;q=0.8,en-US;q=0.7  
        Так же предоставляется следующая информация:
        - User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; 
        - Connection: keep-alive\r\n
        - Accept-Encoding: gzip, deflate\r\n
        - Accept: image/webp,image/apng,image/svg+xml,image/*,*/*;q=0.8\r\n (для запроса на /favicon.ico)
    3.  Адрес компьютера: 192.168.0.104	
    Адрес сервера: 128.119.245.12
    4.  Request uri: /wireshark-labs/HTTP-wireshark-file1.html
    Status code: 200
    5.  Last-Modified: Thu, 16 Feb 2023 06:59:02 GMT\r\n
    6.  Content-Length: 128\r\n
    ![alt text](./1.png)
2.  
    1.  Нет
    2.  Содержимое вернулось явно, т.к. Content-Length: 371\r\n
    3.  Да, If-Modified-Since: Thu, 16 Feb 2023 06:59:02 GMT\r\n
    4.  304 Not Modified
        Содержимое не вернулось, т.к. Content-Length на этот раз отсутствует
    ![alt text](./2.png)
3.
    1.  1 сообщение. Frame Number: 76
    2.  Frame Number: 94
    3.  2 ([2 Reassembled TCP Segments (4861 bytes): #93(4380), #94(481)])
    4.  Нет
    ![alt text](./3.png)
4.  1.  http://gaia.cs.umass.edu/wireshark-labs/HTTP-wireshark-file4.html,  
    http://gaia.cs.umass.edu/pearson.png,  
    http://kurose.cslash.net/8E_cover_small.jpg
    2.  Параллельно - видно, что сначала произошли 2 GET запроса, а затем пришли 2 ответа на них, следовательно, браузер не дожидался ответа на первый запрос и сразу же отправил второй.
    ![alt text](./4.png)
5.  1.  401 Unauthorized
    2.  Authorization: Basic d2lyZXNoYXJrLXN0dWRlbnRzOm5ldHdvcms=\r\n
    Credentials: wireshark-students:network
    ![alt text](./5.png)



 