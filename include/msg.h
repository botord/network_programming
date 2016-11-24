/*************************************************************************
	> File Name: msg.h
	> Author: Dong Hao 
	> Mail: haodong.donh@gmail.com
	> Created Time: Thu 24 Nov 2016 10:59:56 AM CST
 ************************************************************************/

#ifndef _MSG_H
#define _MSG_H
#include <sys/types.h>

typedef struct {
    //协议头
    char head[50];
    //校验码
    unsigned int checksum;
    //协议体
    char buff[1024];    //数据
}msg;

/*发送一个基于自定义协议的message,数据存放在buff中*/
extern int write_msg(int sockfd, char *buff, size_t len);

/*读取一个基于自定义协议的message,数据读取到buff中*/
extern int read_msg(int sockfd, char *buff, size_t len);

#endif
