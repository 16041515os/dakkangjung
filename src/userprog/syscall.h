#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "lib/user/syscall.h"

void syscall_init (void);

////////구현해야할것들///////////
void halt(void);
void exit(int status);
pid_t exec(const char *cmd_line);
int wait(pid_t pid);
int read(int fd, void *buffer, unsigned size);
int write(int fd, const void* buffer, unsigned size);
int pibonacci(int n);
int sum_of_four_integers(int a, int b, int c, int d);

#endif /* userprog/syscall.h */
