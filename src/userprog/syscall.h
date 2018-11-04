#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "lib/user/syscall.h"

void syscall_init (void);

////////구현해야할것들///////////
void halt(void);
void exit(int status);
pid_t exec(const char *cmd_line);
int wait(pid_t pid);
int pibonacci(int n);
int sum_of_four_integers(int a, int b, int c, int d);

/*** project 2 ***/
int read(int fd, void *buffer, unsigned size);
int write(int fd, const void* buffer, unsigned size);
bool create(const char *file, unsigned initial_size);
bool remove(const char *file);
int open(const char *file);
int filesize(int fd);
void seek(int fd, unsigned position);
unsigned tell(int fd);
void close(int fd);
/*****************/

#endif /* userprog/syscall.h */
