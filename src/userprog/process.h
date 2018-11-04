#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"

#define FD_MAX ((1<<(PGBITS-2))-1)

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

int process_open_file(char const *fname);
bool process_close_file(int fd);
struct file *process_get_file(int fd);

#endif /* userprog/process.h */
