/*********************************************************************
	> File Name: src/tcp_client.c
	> Author: Dong Hao
	> Mail: haodong.donh@gmail.com 
	> Created Time: Wed 14 Dec 2016 03:47:01 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <memory.h>
#include <signal.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int sockfd;

void sig_handler(int signo)
{
    if (signo == SIGINT) {
        char *info = "server close";
        write(STDOUT_FILENO, info, sizeof(info));
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

static unsigned int get_ip_by_name(char *name)
{
    unsigned int ip = 0;
    struct hostent *h;

    /*循环遍历/etc/hosts里的所有信息*/
    while ((h = gethostent()) != NULL) {
        if (is_host(h, name)) {
            memcpy(&ip, h->h_addr_list[0], 4);
            break;
        }
    }

    return ip;
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        printf("Usage: %s ip port", argv[0]);
        exit(1);
    }

    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        perror("sig int error");
        exit(1);
    }

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(atoi(argv[2]));
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
    size_t size;
    const char *prompt = "> ";
    strcpy(buffer, "welcome to www.haodong.org");
    write(STDOUT_FILENO, buffer, sizeof(buffer));

    while (1) {
        write(STDOUT_FILENO, prompt, sizeof(prompt));
        size = read(STDIN_FILENO, buffer, sizeof(buffer));
        if (size < 0) {
            continue;
        }
        buffer[size - 1] = '\0';

        if (size = write(sockfd, buffer, size) < 0) {
            perror("write error");
            continue;
        } else {
            memset(buffer, 0, sizeof(buffer));
            if (read(sockfd, buffer, sizeof(buffer)) < 0) {
                perror("read error");
                continue;
            } else {
                printf("%s\n", buffer);
            }
        }
    }

    return 0;
}

