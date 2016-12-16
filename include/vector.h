/*************************************************************************
	> File Name: vector.h
	> Author: 
	> Mail: 
	> Created Time: Wed 14 Dec 2016 11:08:40 AM CST
 ************************************************************************/

#ifndef _VECTOR_H
#define _VECTOR_H
typedef struct {
    int *fd;
    int counter;
    int max_counter;
}VectorFd;

extern VectorFd* create_vector_fd(void);
extern void destroy_vector_fd(VectorFd *);
extern int get_fd(VectorFd *, int index);
extern void add_fd(VectorFd *, int fd);
extern void remove_fd(VectorFd *, int fd);
extern void print_fd(VectorFd *);

#endif
