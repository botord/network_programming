Network Programming practice.

COMPLETED

--使用自定义协议 
--服务器多进程支持
gcc -o obj/msg.o -Iinclude -c src/msg.c 
gcc -o bin/client -Iinclude obj/msg.o src/client.c
gcc -o bin/server -Iinclude obj/msg.o src/server.c

TODO
--服务器多线程支持
