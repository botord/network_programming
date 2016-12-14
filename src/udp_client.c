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
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>


/*
*   struct hostent {
*       char *h_name;       // 主机规范名
*       char **h_aliases;   // 主机别名
*       int h_addrtype;     // 主机IP地址类型 IPv4/IPv6
*       int h_length;       // 地址长度
*       char **h_addr_list; //
*   }
*
*   #include <netdb.h>
*   struct hostent *gethostent();
*   struct hostent *gethostbyname(const char *hostname);
*   void sethostent(int stayopen);
*   void endhostent();
*   
*   192.168.1.1     netgear netgear5g netgear88
*   gethostent() 调用后，返回的host结构
*   struct hostent {
*       char *h_name;       //"netgear"
*       char **h_aliases;   //"netgear5g","netgear88"
*       int h_addrtype;     //IPv4
*       int h_length;       //
*       char **h_addr_list; //
*   }
*
*   gethostbyname("www.baidu.com");
*   //返回该域名所在行的信息
*
*
* */
int sockfd;

void sig_int_handler(int signal)
{
    if (signal == SIGINT) {
        printf("client close\n");
        close(sockfd);
        exit(1);
    }
}

int is_host(struct hostent *h, char *name)
{
    if (!strcmp(h->h_name, name)) return 1;
    int i = 0;
    while (h->h_aliases[i] != NULL) {
        if (!strcmp(h->h_aliases[i], name)) return 1;
        i++;
    }

    return 0;
}

unsigned int get_ip_by_name(char *name) 
{
    unsigned int ip = 0;
    struct hostent *h;

    /*循环遍历/etc/hosts里所有的信息*/
    while ((h = gethostent()) != NULL) {
        if (is_host(h, name)) {
            memcpy(&ip, h->h_addr_list[0], 4);
            break;
        }

    }

    endhostent();
    return ip;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s #port\n", argv[0]);
        exit(1);
    }

    if (signal(SIGINT, sig_int_handler) == SIG_ERR) {
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
    /*
    inet_pton(AF_INET, argv[1], &serveraddr.sin_addr.s_addr);//address
    */
    
    unsigned int ip = get_ip_by_name(argv[1]);
    if (ip != 0) {
        serveraddr.sin_addr.s_addr = ip;
    } else {
        inet_pton(AF_INET, argv[1], &serveraddr.sin_addr.s_addr);
    }
    if (connect(sockfd, (struct sockaddr *)&serveraddr, 
                sizeof(serveraddr)) < 0) {
        perror("connect error");
        exit(1);
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
