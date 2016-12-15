/*************************************************************************
	> File Name: vector.c
	> Author: Dong Hao 
	> Mail: haodong.donh@gmail.com
	> Created Time: Wed 14 Dec 2016 11:08:40 AM CST
 ************************************************************************/
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "vector.h"

static void encapacity(VectorFd *vfd)
{
    if (vfd->counter >= vfd->max_counter) {
        int *fds = (int *) calloc(vfd->counter + 5, sizeof(int));
        /*
        int *fds = vfd->fd;
        vfd->fd = realloc(vfd->fd, vfd->counter + 5);
        if (vfd->fd == NULL) {
            vfd->fd = fds;
        }
        */
        assert(fds != NULL);
        memcpy(fds, vfd->fd, sizeof(int) * vfd->counter);

        free(vfd->fd);
        vfd->fd = fds;
        vfd->max_counter += 5;
    }
}

VectorFd* create_vector_fd(void)
{
    VectorFd *vfd = (VectorFd *)calloc(1, sizeof(VectorFd));
    assert(vfd != NULL);

    vfd->fd = (int *)calloc(5, sizeof(int));
    assert(vfd->fd != NULL);

    vfd->counter = 0;
    vfd->max_counter = 5;
}

void destroy_vector_fd(VectorFd *vfd)
{
    assert(vfd != NULL);
    free(vfd->fd);
    free(vfd);
}

int get_fd(VectorFd *vfd, int index)
{
    assert(vfd != NULL);

    if (index < 0 || index > vfd->counter -1) {
        return 0;
    }

    return vfd->fd[index];
}


void add_fd(VectorFd *vfd, int fd)
{
    assert(vfd != NULL);
    if (vfd->counter >= vfd->max_counter) {
        encapacity(vfd);
    }

    vfd->fd[vfd->counter++] = fd;

}

static int indexof(VectorFd *vfd, int fd)
{
    int i = 0;
    for (;i < vfd->counter; i++) {
        if (vfd->fd[i] == fd) return i;
    }

    return -1;
}

void remove_fd(VectorFd *vfd, int fd)
{
    assert(vfd != NULL);

    int index = indexof(vfd, fd);
    if (index == -1) {
        return;
    }

    int i = index;
    for (; i < vfd->max_counter; i++) {
        vfd->fd[i] = vfd->fd[i+1];
    }
}

void print_fd(VectorFd *vfd)
{
    int counter = vfd->counter;
    int max_counter = vfd->max_counter;

    printf("vfd->max_counter = %d\n", vfd->max_counter);
    printf("vfd->counter = %d\n", vfd->counter);

    int i = 0;
    for (; i<counter; i++) {
        printf("vfd->fd[%d] = %d\t", i, vfd->fd[i]);
    }

    printf("\n");
}
