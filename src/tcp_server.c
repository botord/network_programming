/*************************************************************************
	> File Name: src/tcp_server.c
	> Author: Dong Hao 
	> Mail: haodong.donh@gmail.com
	> Created Time: Wed 14 Dec 2016 11:54:32 AM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <memory.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "vector.h"


VectorFd *vfd;

int sockfd;

void sig_handler(int signo)
{
    if (signo == SIGINT) {
        printf("server close\n");
        close(sockfd);
        destroy_vector_fd(vfd);
        exit(1);
    }
}

/*
 * fd对应与某个连接的客户端
 * 和某一个连接的客户端进行双向通信（非阻塞）
 * */
void service(int fd) 
{
    char buff[1024];
    size_t size;

    /*非阻塞方式，读不到数据会直接返回，因此不需要判断
    * 返回结果小于0的情况，直接服务下一个客户端*/
    size = read(fd, buff, sizeof(buff));

    if (size == 0) {
        /*客户端关闭连接*/
        char *info = "client closed";
        
        write(STDOUT_FILENO, info, sizeof(info));

        /*删除动态数组中的客户端并关闭描述符*/
        remove_fd(vfd, fd);
        close(fd);
    } else if (size > 0) {
        write(STDOUT_FILENO, buff, sizeof(buff));
        if (write(fd, buff, size) < size) {
            /*客户端关闭连接*/
            if (errno == EPIPE) {
                perror("write error");
                remove_fd(vfd, fd);
                close(fd);
            }
        }
        
    }


    
}

void *th_fn(void *args)
{
   int i;
    while (1) {
        i = 0;
        for (; i < vfd->counter; i++) {
            service(get_fd(vfd, i));
        }
    }

    return (void *)0;
}

void out_addr(struct sockaddr_in *clientaddr)
{
    char ip[16];
    memset(ip, 0, sizeof(ip));
    int port = ntohs(clientaddr->sin_port);
    inet_ntop(AF_INET, 
              &clientaddr->sin_addr.s_addr, ip, sizeof(ip));

    printf("%s(%d) connected\n", ip, port);
}

int main(int argc, char *argv[])
{
    printf("0");
    if (argc < 2) {
        fprintf(stderr, "Usage: %s(port)\n", argv[0]);
        exit(1);
    }

    printf("1");
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        perror("signal sig_int error");
        exit(1);
    }
    printf("2");

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket error");
        exit(1);
    }
    printf("3");

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(atoi(argv[2]));
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    printf("4");

    if (bind(sockfd, (struct sockaddr *)&serveraddr, 
             sizeof(serveraddr)) < 0) {
        perror("bind error");
        exit(1);
    }
    printf("5");
    
    if (listen(sockfd, 10) < 0) {
        perror("listen error");
        exit(1);
    }

    struct sockaddr_in clientaddr;
    socklen_t clientaddr_len = sizeof(clientaddr);

    printf("6");
    /*创建存放fd的动态数组*/
    vfd = create_vector_fd();

    /*设置线程分离属性*/
    pthread_t th;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, 
                                PTHREAD_CREATE_DETACHED);


    int err;
    if ((err = pthread_create(&th, &attr, 
                              th_fn, (void *)0) != 0)) {
        perror("thread create failed");
        exit(1);

    }
    /*
     * 主控线程将获得客户端的连接，并将新的socket描述符放置动态数组中
     * 启动的子线程负责遍历动态数组中socket描述符并和对应的客户端进行
     * 双向通信(非阻塞)
     * */
    while (1) {
        int fd = accept(sockfd, 
                        (struct sockaddr*)&clientaddr, &clientaddr_len);
        if (fd < 0) {
            perror("accept error");
            exit(1);
        }
        out_addr(&clientaddr);
        /*将读写修改为非阻塞方式*/
        int val;
        fcntl(fd, F_GETFL, &val);
        val |= O_NONBLOCK;

        fcntl(fd, F_SETFL, val);

        /*将返回的fd加入到之前创建的动态数组中*/
        add_fd(vfd, fd);
    }
}
