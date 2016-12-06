/*************************************************************************
	> File Name: udp_client.c
	> Author: 
	> Mail: 
	> Created Time: Tue 06 Dec 2016 02:51:52 PM CST
 ************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <memory.h>
#include <arpa/inet.h>
#include <netinet/in.h>


int sockfd;

void sig_int_handler(int signal)
{
    if (signal == SIGINT) {
        printf("client close\n");
        close(sockfd);
        exit(1);
    }
}
int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s #port\n", argv[0]);
        exit(1);
    }

    if (signal(SIGINT, sig_int_handler) < 0) {
        perror("sigal SIGINT error");
        exit(1);
    }

    /*1. 创建socket*/
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket error");
        exit(1);
    }

    /*2. 调用recvfrom sendto和服务器进行双向通信*/
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(atoi(argv[2]));//port
    inet_pton(AF_INET, argv[1], &serveraddr.sin_addr.s_addr);//address

    if (connect(sockfd, (struct sockaddr *)&serveraddr, 
                sizeof(serveraddr)) < 0) {
        perror("connect error");
    }

    char buffer[1024];
    strcpy(buffer, "hello world.");
    /*
    if (sendto(sockfd, buffer, sizeof(buffer), 0,
               (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
    */
    if (send(sockfd, buffer, sizeof(buffer), 0) < 0) {
        perror("send to server error");
        exit(1);
    } else {
        memset(buffer, 0, sizeof(buffer));
        //接收报文无需再指定serveraddr信息
        if (recv(sockfd, buffer, sizeof(buffer), 0) < 0) {
            perror("recv error");
            exit(1);
        } else {
            printf("%s", buffer);
        }
    }



}
