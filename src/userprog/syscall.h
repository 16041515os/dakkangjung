#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "threads/thread.h"

void syscall_init (void);

////////구현해야할것들///////////
void halt(void);
void exit(int status);
tid_t exec(const char *cmd_line);
int wait(tid_t pid);
int pibonacci(int n);
int sum_of_four_integers(int a, int b, int c, int d);

#endif /* userprog/syscall.h */
