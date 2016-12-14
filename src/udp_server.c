/*************************************************************************
	> File Name: src/udp_client.c
	> Author: Dong Hao
	> Mail: haodong.donh@gmail.com
	> Created Time: Fri 02 Dec 2016 05:41:33 PM CST
 ************************************************************************/

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <string.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <pthread.h>

int sockfd;

void sig_int_handler(int signo) {
    if (signo == SIGINT) {
        printf("server close.\n");
        close(sockfd);
        exit(1);
    }
}

void out_addr(int clientfd)
{
    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);

    if (getpeername(clientfd, (struct sockaddr *)&clientaddr, &len) < 0) {
        perror("get peername error");
        exit(1);
    }

    char ip[16];
    memset(ip, 0, sizeof(ip));
    inet_ntop(AF_INET, &clientaddr.sin_addr.s_addr, 
              ip, sizeof(ip));
    int port = ntohs(clientaddr.sin_port);
    printf("client: %s: %d\n", ip, port);

}

void service(int fd) 
{
    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);
    char buff[1024];
    memset(buff, 0, sizeof(buff));
    //接收客户端的数据报文
    //为防止客户端断开连接后继续等待，可以设置超时控制，套接字选项里也
    //可以进行设置
    if (recvfrom(fd, buff, sizeof(buff), 0, 
                 (struct sockaddr *)&clientaddr, &len) < 0) {
        perror("recvfrom client error");
    } else {
        printf("client send info: %s\n", buff);
        
        long int t = time(0);
        char *ptr = ctime(&t);

        size_t size = strlen(ptr) * sizeof(char);
        if (sendto(fd, ptr, size, 0, 
            (struct sockaddr *)&clientaddr, 
            sizeof(clientaddr)) < 0) {
            perror("sendto client error");               
        }
    }
}

/*
void *thread_fn(void *clientfd)
{
    int fd = *(int *)clientfd;
    out_addr(fd);
    service(fd);
    close(fd);
}
*/
int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s #port\n", argv[0]);
        exit(1);
    }

    if (signal(SIGINT, sig_int_handler) == SIG_ERR) {
        perror("signal SIGINT error");
        exit(1);
    }

    /*1. 创建socket*/
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("sockfd create failed.");
        exit(0);
    }

    int ret;
    int opt = 1;

    //设置套接字选项为SO_REUSEADDR,保证主程序退出重新绑定
    //套接字端口时可以立即使用
    if (ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
        &opt, sizeof(opt)) < 0) {
        perror("setsockopt error");    
        exit(1);
    }

    /*2. 调用bind对socket和地址进行绑定*/
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(atoi(argv[1]));
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (struct sockaddr *)&serveraddr, 
                            sizeof(serveraddr)) < 0) {
        perror("bind error");
    }

    /*3. 双向通信*/
    while (1) {
        service(sockfd);
    }


}
