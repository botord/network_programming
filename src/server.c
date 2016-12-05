#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "msg.h"

int sockfd;

void sig_int_handler(int signo)
{
	if (signo == SIGINT) {
		printf("Server close\n");
		close(sockfd);
		exit(1);
	}
}

//输出连接的客户端相关信息
void print_addr(struct sockaddr_in *clientaddr) {
	int port = ntohs(clientaddr->sin_port);
	char ip[16];

	memset(ip, 0, sizeof(ip));
	//将ip地址从网络字节序转换成点分十进制
	inet_ntop(AF_INET, &clientaddr->sin_addr.s_addr, ip, sizeof(ip));

	printf("Client: %s: %d connected\n", ip, port);

}

void service(int fd)
{
    //和客户端进行读写操作，双向通信
    char buff[1024];

    printf("starting IO..\n");
    while (1) {
        memset(buff, 0, sizeof(buff));

        size_t size;
        if ((size = read_msg(fd, buff, sizeof(buff))) < 0) {
            perror("read_msg error");
            break;
        } else if (size == 0) {
            //如果读取结果是0，说明客户端已断开连接，这个时候直接退出
            printf("client closed, will exit.\n");
            break;
        } else {
            //write(STDOUT_FILENO, buff, sizeof(buff));
            printf("%s\n", buff);

            if ((write_msg(fd, buff, sizeof(buff)) < 0)) {
                if (errno == EPIPE) {//管道错误
                    break;
                }
                perror("protocol error");
            }
        }
    }
}

//通过fd获取客户端相关信息
void out_fd(int fd)
{
    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);
    if (getpeername(fd, (struct sockaddr *)&clientaddr, &len) < 0) {
        perror("getpeername failed");
        return;
    }

    char ip[16];
    memset(ip, 0, sizeof(ip));
    int port = ntohs(clientaddr.sin_port);
    inet_ntop(AF_INET, &clientaddr.sin_addr.s_addr, ip, sizeof(ip));
    printf("%16s: %5d closed\n", ip, port);
}

void * thread_fn(void *arg)
{
    int fd = (int *)arg;
    service(fd);
    out_fd(fd);
    close(fd);
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("Usage: %s #port\n", argv[0]);
		exit(1);
	}

	/* 注册中断处理函数 */
	if (signal(SIGINT, sig_int_handler) == SIG_ERR) {
		perror("signal sig_int error");
		exit(1);
	}

	/* 1. 创建套接字 */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("socket error");
		exit(1);
	}

	printf("socket(%d) created.. \n", sockfd);

	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	//IPV4
	serveraddr.sin_family = AF_INET;
	//port
	serveraddr.sin_port = htons(atoi(argv[1])); 
	printf("port is %d.\n", atoi(argv[1]));
	/*
	 * struct sockaddr_in {
	 * 	short int sin_family;		//Internet地址族 AF_INET
	 * 	unsigned short int sin_port;	//端口号 16位
	 * 	struct in_addr sin_addr;	//Internet地址，32位
	 * 	unsigned char sin_zero[8];
	 * }
	 *
	 * struct in_addr {
	 *	in_addr_t s_addr;		//IPV4地址
	 * }
	 *
	 * */
	//serveraddr.sin_addr.s_addr = '127.0.0.1';
	//#define INADDR_ANY (uint32_t)0x00000000
	//响应本机所有网卡的请求
	serveraddr.sin_addr.s_addr = INADDR_ANY;
	/* 2. 调用bind函数把socket和地址进行绑定 */
	if (bind(sockfd, (struct sockaddr *)&serveraddr, 
				sizeof(serveraddr)) < 0) {
		//要求传入通用的internet地址类型
		perror("bind error");
		exit(1);
	}

    /*3. 开始监听端口*/
    if (listen(sockfd, 10) < 0) {
        perror("listen error");
        exit(1);
    }
    printf("listening...\n");
    struct sockaddr_in clientaddr;
    socklen_t clientaddr_len = sizeof(clientaddr);
    memset(&clientaddr, 0, clientaddr_len);

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

    /*4. 调用accept获取客户端连接,如果没有获得，主进程会阻塞在这里*/

    int err;

	while (1) {
        //主线程不能调用pthread_join，不然会hang在这里
        //套接字里包含了协议信息，服务器端和客户单的IO端口，IP地址
		int fd = accept(sockfd, NULL, NULL);

		if (fd < 0) {
			perror("accept error");
			continue;
		}

        /*5. 以分离状态启动子线程和客户端进行通信*/
        pthread_t thread;
        if ((err = pthread_create(&thread, &attr, thread_fn, 
                                    (void *)fd)) != 0) {
            perror("pthread create failed");
        }

        pthread_attr_destroy(&attr);
    }

	return 0; 
}
