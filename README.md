Network Programming practice.

--使用自定义协议 20161124
--gcc -o obj/msg.o -Iinclude -c src/msg.c 
  gcc -o bin/client -Iinclude obj/msg.o src/client.c
  gcc -o bin/server -Iinclude obj/msg.o src/server.c
