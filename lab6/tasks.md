### 1. Wireshark: UDP
![alt text](./screenshots/wireshark/1.png)
Сделал DNS-запрос.
```console
C:\Users\Astronomax>nslookup www.spbu.ru
╤хЁтхЁ:  UnKnown
Address:  192.168.0.1

╚ь :     spbu.ru
Address:  81.89.183.222
Aliases:  www.spbu.ru
```
![alt text](./screenshots/wireshark/2.png)
Заголовок "Source Port" занимает 2 байта.
![alt text](./screenshots/wireshark/3.png)
"Destination Port" - 2 байта.
![alt text](./screenshots/wireshark/4.png)
"Length" - 2 байта.
![alt text](./screenshots/wireshark/5.png)
"Checksum" - 2 байта.
![alt text](./screenshots/wireshark/6.png)
"UDP payload" - 42 байта.  
Общая сумма сходится с тем, что написано в "Length" (размер UDP дейтаграммы) - 50 байт.  
Максимальный размер полезной нагрузки - 508 байт.  
Максимально возможное значение номера порта отправителя - 65535.
![alt text](./screenshots/wireshark/7.png)
Номер UDP протокола: 17 (11 в шестнадцатеричной системе).  
Номера портов в запросе:  
```
Source Port: 50356
Destination Port: 53
```
Номера портов в ответе:  
```
Source Port: 53
Destination Port: 50356
```
### 2. Программирование. FTP
#### 1. FileZilla сервер и клиент
![alt text](./screenshots/filezilla/1.png)
Запустил FTP-сервер на 21 порту при помощи FileZilla Server.
![alt text](./screenshots/filezilla/2.png)
Добавил пользователя TestUser с паролем 1234, с доступом к каталогу "C:/Astronomax/FileZilla"
![alt text](./screenshots/filezilla/3.png)
Через FileZilla Client подключился к этому серверу.
![alt text](./screenshots/filezilla/4.png)
![alt text](./screenshots/filezilla/5.png)
Добавил через клиент файл "test_file" в удаленный каталог.
![alt text](./screenshots/filezilla/6.png)
Открыл файл на редактирование и внёс следующее содержимое: 
![alt text](./screenshots/filezilla/7.png)
В интерфейсе юзера видим сообщения, что всё прошло успешно.
![alt text](./screenshots/filezilla/8.png)
Так же в приложении администратора сервера видим сообщения об успешных операциях клиента.
![alt text](./screenshots/filezilla/9.png)
Убеждаемся, что теперь в каталоге действительно присутствует переданный по FTP файл. 

  