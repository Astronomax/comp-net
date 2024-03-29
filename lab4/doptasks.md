#### Б. DNS-трассировка www.ietf.org
* *Найдите DNS-запрос и ответ на него. С использованием какого транспортного протокола
  они отправлены?*
  ![alt text](./screenshots/b1.png)
  ![alt text](./screenshots/b2.png)
  Запрос и ответ отправлены с использованием протокола DNS.
* *Какой порт назначения у запроса DNS?*
  ![alt text](./screenshots/b3.png)
  Порт назначения: 53 
* *На какой IP-адрес отправлен DNS-запрос? Используйте ipconfig для определения IP-адреса
  вашего локального DNS-сервера. Одинаковы ли эти два адреса?*  
  Запрос отправлен на 192.168.0.1
  ![alt text](./screenshots/b4.png)
* *Проанализируйте сообщение-запрос DNS. Запись какого типа запрашивается? Содержатся
  ли в запросе какие-нибудь «ответы»?*
  ![alt text](./screenshots/b5.png)
  Запрашивается запись типа A. В запросе 0 ответов.  
* *Проанализируйте ответное сообщение DNS. Сколько в нем «ответов»? Что содержится в
  каждом?*
  ![alt text](./screenshots/b6.png)
  В ответном сообщении 3 ответа.
* *Посмотрите на последующий TCP-пакет с флагом SYN, отправленный вашим компьютером.
  Соответствует ли IP-адрес назначения пакета с SYN одному из адресов, приведенных в
  ответном сообщении DNS?*
  ![alt text](./screenshots/b7.png)  
  Да, соответствует.
* *Веб-страница содержит изображения. Выполняет ли хост новые запросы DNS перед
  загрузкой этих изображений?*  
  Нет, не выполняет.
#### В. DNS-трассировка www.spbu.ru
* *Каков порт назначения в запросе DNS? Какой порт источника в DNS-ответе?*
  ![alt text](./screenshots/c1.png)
  Порт назначения в запросе и порт источника в ответе совпадают: 53.
* *На какой IP-адрес отправлен DNS-запрос? Совпадает ли он с адресом локального DNSсервера, установленного по умолчанию?*
  Запрос отправлен на 192.168.0.1. Да, совпадает.
* *Проанализируйте сообщение-запрос DNS. Запись какого типа запрашивается? Содержатся
   ли в запросе какие-нибудь «ответы»?*  
  Запрашивается запись типа AAAA. В запросе 0 ответов.
* *Проанализируйте ответное сообщение DNS. Сколько в нем «ответов»? Что содержится в
   каждом?*
   ![alt text](./screenshots/c2.png)
   В ответном сообщении 1 ответ.
#### Г. DNS-трассировка nslookup –type=NS
* *На какой IP-адрес отправлен DNS-запрос? Совпадает ли он с адресом локального DNSсервера, установленного по умолчанию?*
  ![alt text](./screenshots/d1.png)
  Запрос отправлен на 192.168.0.1. Да, совпадает.
* *Проанализируйте сообщение-запрос DNS. Запись какого типа запрашивается? Содержатся
   ли в запросе какие-нибудь «ответы»?*
  ![alt text](./screenshots/d2.png)
  Запрашивается запись типа NS. В запросе 0 ответов.
* *Проанализируйте ответное сообщение DNS. Имена каких DNS-серверов университета в
   нем содержатся? А есть ли их адреса в этом ответе?*  
  ![alt text](./screenshots/d3.png)
#### Д. DNS-трассировка nslookup www.spbu.ru ns2.pu.ru
* *На какой IP-адрес отправлен DNS-запрос? Совпадает ли он с адресом локального DNSсервера, установленного по умолчанию? Если нет, то какому хосту он принадлежит?*  
  ![alt text](./screenshots/e1.png)
  Нет, не совпадает: 195.70.196.210.
* *Проанализируйте сообщение-запрос DNS. Запись какого типа запрашивается? Содержатся
   ли в запросе какие-нибудь «ответы»?*  
   ![alt text](./screenshots/e2.png)
   Запрашивается запись типа AAAA. В запросе 0 ответов.
* *Проанализируйте ответное сообщение DNS. Сколько в нем «ответов»? Что содержится в
   каждом?*  
   В ответном сообщении 1 ответ.
  

    
