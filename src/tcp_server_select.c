/*************************************************************************
	> File Name: src/tcp_server_select.c
	> Author: Dong Hao 
	> Mail: haodong.donh@gmail.com
	> Created Time: Mon 19 Dec 2016 10:32:48 AM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

#include "vector.h"

void *th_fn(void *arg);
void out_addr(struct sockaddr_in *);
void service(int fd);

void sig_handler(int signo)
{
    if (signo == SIGINT){
        perror("server close");
        exit(1);
    }
}

int sockfd;
VectorFd *vfd;

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s(port)", argv[0]);
    }

    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        perror("signal error");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket error");
        exit(1);
    }

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(atoi(argv[1]));
    serveraddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&serveraddr, 
                          sizeof(serveraddr)) < 0) {
        perror("bind error");
        exit(1);
    } 

    if (listen(sockfd, 10) < 0) {
        perror("listen error");
        exit(1);
    }

    struct sockaddr_in clientaddr;
    socklen_t socketlen = sizeof(clientaddr);
    vfd = create_vector_fd();

    pthread_t th;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, 
                                PTHREAD_CREATE_DETACHED);


    int err;
    if ((err = pthread_create(&th, &attr, 
                             th_fn, (void *)0)) != 0) {
        perror("thread create error");
        exit(1);
    }

    pthread_attr_destroy(&attr);


    /*
     * 1. 主控线程获得客户端的连接，将新的socket描述符放到动态数组中
     * 2. 启动的子线程调用select函数委托内核检查传入到select中的描述
     * 符是否已经准备好
     * 3. 利用FD_ISSET找出准备好的那些描述符并和对应的客户端进行双向
     * 通信(非阻塞)
     * */
    
    while (1) {
        int fd = accept(sockfd, (struct sockaddr *)&clientaddr,
                                                &socketlen);

        if (fd < 0) {
            perror("accept error");
            continue;
        }

        out_addr(&clientaddr);
        add_fd(vfd, fd);
    }
}
/* 遍历动态数值中所有的描述符并加入描述符集，同时此函数返回动态数
 * 组中最大的描述符
 * */
int add_set(fd_set *set) 
{
    FD_ZERO(set);
    int max_fd = vfd->fd[0];

    int i = 0;
    for (; i< vfd->counter; i++) {
        int fd = get_fd(vfd, i);
        if (fd > max_fd) {
            max_fd = fd;
        }
        FD_SET(fd, set);
    }

    return max_fd;
}

void *th_fn(void *arg)
{
    struct timeval t;
    //秒
    t.tv_sec = 2;
    //毫秒
    t.tv_usec = 0;
    //准备好描述符的数量
    int n = 0;
    //所有描述符的最大值
    int maxfd = 0;
    //描述符集
    fd_set set;

    maxfd = add_set(&set);

    /* 
     * 调用select函数会阻塞，委托内核去检查传入的描述符是否准备好，
     * 若有则返回准备好的描述符，超时则返回０
     * 第一个参数为描述符集中描述符的范围(最大描述符+1)
     * */
    while ((n = select(maxfd+1, &set, NULL, NULL, &t)) >= 0) {
        if (n > 0) {

            /* 
             * 检测哪些描述符已经准备好，并和这些已准备好的描述符
             * 对应的客户端进行双向通信
             * */
            int i = 0;
            for (; i < vfd->counter; i++) {
                int fd = get_fd(vfd, i);
                if (FD_ISSET(fd, &set)) {
                    service(fd);
                }
            }
            
        } else {
            //超时
            t.tv_sec = 2;
            t.tv_usec = 0;
            //重新遍历动态数组
            maxfd = add_set(&set);
        }
        
    }
    
}

void service(int fd)
{
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    size_t size = read(fd, buffer, sizeof(buffer)); 

    if (size == 0) {
        //printf("client closed");
        write(STDOUT_FILENO, "client closed\n", 15);
        remove_fd(vfd, fd);
        close(fd);
    } else if (size > 0) {
        write(STDOUT_FILENO, buffer, sizeof(buffer));
        //printf("%s\n", buffer);

        if (write(fd, buffer, sizeof(buffer)) != size) {
            if (errno == EPIPE) {
                perror("write error");
                remove_fd(vfd, fd);
                close(fd);
            }
        }
    }
}

void out_addr(struct sockaddr_in *sockaddr)
{
    char ip[16];
    memset(ip, 0, sizeof(ip));

    inet_ntop(sockaddr->sin_family, 
              &sockaddr->sin_addr.s_addr, ip, sizeof(ip));

    int port = ntohs(sockaddr->sin_port);
    printf("%s(%d) connected\n",ip, port);
}

