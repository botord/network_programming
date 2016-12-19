Network Programming practice.

COMPLETED

--使用自定义协议

--服务器多进程支持

--gcc -o obj/msg.o -Iinclude -c src/msg.c 

--gcc -o bin/client -Iinclude obj/msg.o src/client.c

--gcc -o bin/server -Iinclude obj/msg.o src/server.c

--服务器多线程支持

--服务器端启动线程遍历已连接的客户端并以非阻塞方式进行读写处理

--主线程监听端口，把连接放入描述符集，子线程遍历描述符集，并进行双向通信

--子线程调用select委托内核检查传入的描述符是否准备好，根据返回的已准备好的描述符，进行IO操作

TODO

--select/poll model on server
