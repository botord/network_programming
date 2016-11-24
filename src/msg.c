/*************************************************************************
	> File Name: msg.h
	> Author: Dong Hao 
	> Mail: haodong.donh@gmail.com
	> Created Time: Thu 24 Nov 2016 10:59:56 AM CST
 ************************************************************************/

#include "msg.h"
#include <unistd.h>
#include <memory.h>
#include <stdio.h>
#include <sys/types.h>


/* Check the head of message.*/
static unsigned int msg_check(msg *message)
{
    unsigned int ret = 0;
    int i;

    for (i=0; i<sizeof(message->head); i++) {
        ret += message->head[i];
    }

    return ret;
}

int write_msg(int sockfd, char *buff, size_t len) {
    msg message;
    memset(&message, 0, sizeof(message));
    strcpy(message.head, "haodong.org");
    memcpy(message.buff, buff, len);
    message.checksum = msg_check(&message); 
    size_t size;
    if (size = write(sockfd, &message,
                     sizeof(message)) != sizeof(message)) {
        return -1;
    }

    return size;
}

int read_msg(int sockfd, char *buff, size_t len) {
    msg message;
    memset(&message, 0, sizeof(message));

    size_t size;

    if((size = read(sockfd, &message, sizeof(message))) < 0) {
        return -1;
    } else if (size == 0) {
        //数据读取完毕
        return 0;
    }

    //读取成功
    unsigned int checksum = msg_check(&message);

    if ((checksum == message.checksum) && 
        (!strcmp(message.head, "haodong.org"))) {
            memcpy(buff, message.buff, len);
            return sizeof(message);
        }

    //校验失败
    printf("checksum failed.\n");
    return -1;
}
