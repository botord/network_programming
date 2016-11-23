#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>

int sockfd;

void sig_handler(int signo)
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
	long t = time(0);
	char *s = ctime(&t);
	size_t size =  strlen(s) * sizeof(char);

	//将从服务器端获得的系统时间写回到客户端
	
	if (write(fd, s, size) != size) {
		perror("write error");
	}
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("Usage: %s #port\n", argv[0]);
		exit(1);
	}

	/* 注册中断处理函数 */
	if (signal(SIGINT, sig_handler) == SIG_ERR) {
		perror("signal sigint error");
		exit(1);
	}

	/* 1. 创建套接字 */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	//IPV4
	serveraddr.sin_family = AF_INET;
	//port
	serveraddr.sin_port = htons(atoi(argv[1])); 
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
	//相应本机所有网卡的请求
	serveraddr.sin_addr.s_addr = INADDR_ANY;
	/* 2. 调用bind函数把socket和地址进行绑定 */
	if (bind(sockfd, (struct sockaddr *)&serveraddr, 
				sizeof(serveraddr)) < 0) {
		//要求传入通用的internet地址类型
		perror("bind error");
		exit(1);
	}

	/*3. 使用listen开始监听来自客户端的端口, 指定port监听，
	 * 通知系统接收来自客户端的连接请求*/
	if (listen(sockfd, 10) < 0) {
		//服务器端存放一个队列，将接收到的客户端连接
		//请求放到对应的队列当中
		perror("listen error");
		exit(1);
	}

	/*4. 通过accept从队列中获得一个客户端的请求连接,并返回新的socket描述符。*/
	// accept() 获取连接和连接对应的描述符
	struct sockaddr_in clientaddr;
	socklen_t clientaddr_len = sizeof(clientaddr);
	while (1) {
		int fd = accept(sockfd, (struct sockaddr *)&clientaddr,
						&clientaddr_len);
		//如果没有客户端连接，程序会阻塞在accept中，直到获得客户端连接

		if (fd < 0) {
			perror("accept error");
			continue;
		}

		/*5. 调用io函数，read/write和客户端进行通信 */
		print_addr(&clientaddr);
		service(fd);

		/*关闭socket*/
		close(fd);
	}


	return 0; 
}
