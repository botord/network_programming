#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <memory.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>



int main(int argc, char *argv[])
{
	if (argc < 3) {
		printf("usage: %s ip port\n", argv[0]);
		exit(1);
	}

	/*1. 创建套接字*/
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("socket error");
		exit(1);
	}

	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));

	/*向serveraddr中填充ip port和地址族类型(ipv4)*/
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &serveraddr.sin_addr.s_addr);

	/*2. 调用connect函数连接服务器*/
	if (connect(sockfd, (struct sockaddr *)&serveraddr,
			       	sizeof(serveraddr)) < 0) {
		perror("connect error");
		exit(1);
	}


	/*3. 进行IO操作*/
	char buffer[1024];
	memset(buffer, '\0', sizeof(buffer));

	size_t size;
	if (size = read(sockfd, buffer, sizeof(buffer)) < 0) {
		perror("read error");
	}
	printf("buffer: %s", buffer);
	if (write(STDOUT_FILENO, buffer, size) != size) {
		perror("write error");
	}

	return 0;
}
