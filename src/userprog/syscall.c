#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "devices/shutdown.h"
#include "threads/vaddr.h"
#include "userprog/process.h"
#include "devices/input.h"
#include "filesys/file.h"
#include "filesys/filesys.h"

static struct lock file_lock;

static void vaddr_check(void* addr){// check valid address
  if(!is_user_vaddr(addr)) exit(-1);
}

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&file_lock);
}

static void
syscall_handler (struct intr_frame *f) 
{ 
  
  int SYSCALL_NUM;
  int n,n1,n2,n3,n4;
  const char* _cline;

  vaddr_check(f->esp);
  SYSCALL_NUM = *(int*)(f->esp);

  if(SYSCALL_NUM == SYS_HALT){
      halt();
  }

  else if(SYSCALL_NUM == SYS_EXIT){

    vaddr_check(f->esp + 4);
    exit(*(uint32_t*)(f->esp+4));
  }

  else if(SYSCALL_NUM == SYS_EXEC){

    _cline = (const char*)*(uint32_t*)(f->esp + 4);
    vaddr_check((void*)_cline);
    f->eax = exec(_cline);
  }

  else if(SYSCALL_NUM == SYS_WAIT){
  
   f->eax =  wait((int)*(uint32_t*)(f->esp+4));
  
  }

  else if(SYSCALL_NUM == SYS_CREATE) {
    vaddr_check(f->esp+8);

    char const *fname;
    unsigned size;

    fname = (char *)*(uint32_t *)(f->esp+4);
    if(!is_user_vaddr(fname)) exit(-1);
    UNUSED volatile const char _ = *fname; // try access

    size = *(uint32_t*)(f->esp+8);

    f->eax = filesys_create(fname, size);
  }

  else if(SYSCALL_NUM == SYS_REMOVE) {
    vaddr_check(f->esp+4);

    char const *fname;
    fname = (char *)*(uint32_t *)(f->esp+4);
    if(!is_user_vaddr(fname)) exit(-1);
    UNUSED volatile const char _ = *fname; // try access

    lock_acquire(&file_lock);
    f->eax = filesys_remove(fname);
    lock_release(&file_lock);
  }

  else if(SYSCALL_NUM == SYS_OPEN) {
    vaddr_check(f->esp+4);

    const char *fname;
    fname = (char *)*(uint32_t *)(f->esp+4);
    if(!is_user_vaddr(fname)) exit(-1);
    UNUSED volatile const char _ = *fname; // try access

    f->eax = process_open_file(fname);
  }

  else if(SYSCALL_NUM == SYS_FILESIZE) {
    vaddr_check(f->esp+4);

    int fd = *(uint32_t*)(f->esp+4);
    struct file *file = process_get_file(fd);
    if(file == NULL) exit(-1);

    f->eax = file_length(file);
  }

  else if(SYSCALL_NUM == SYS_READ){
    vaddr_check(f->esp+12);

    int fd;
    uint8_t *buffer;
    unsigned size;
    fd = *(uint32_t *)(f->esp+4);
    buffer = (uint8_t *)*(uint32_t *)(f->esp+8);
    size = *(uint32_t *)(f->esp+12);

    if(fd == 1 || fd == 2 || !is_user_vaddr(buffer)) {
      thread_exit(-1);
    }
    else if(fd == 0) {
      unsigned i;
      lock_acquire(&file_lock);
      for(i = 0; i < size; i++){
        buffer[i] = input_getc();
      }
      lock_release(&file_lock);
      f->eax = size;
    }
    else {
      struct file *file = process_get_file(fd);
      if(file == NULL) thread_exit(-1);

      lock_acquire(&file_lock);
      f->eax = file_read(file, buffer, size);
      lock_release(&file_lock);
    }
  }

  else if(SYSCALL_NUM == SYS_WRITE){
    vaddr_check(f->esp+12);

    int fd;
    void *buffer;
    unsigned size;
    fd = *(uint32_t *)(f->esp+4);
    buffer = (void *)*(uint32_t *)(f->esp+8);
    size = *(uint32_t *)(f->esp+12);

    if(fd == 0 || !is_user_vaddr(buffer)) {
      thread_exit(-1);
    }
    else if(fd == 1 /*|| fd == 2*/) {
      if(!is_user_vaddr(buffer)) thread_exit(-1);
      lock_acquire(&file_lock);
      putbuf(buffer, size);
      lock_release(&file_lock);
      f->eax = size;
    }
    else {
      struct file *file = process_get_file(fd);
      if(file == NULL) thread_exit(-1);

      lock_acquire(&file_lock);
      f->eax = file_write(file, buffer, size);
      lock_release(&file_lock);
    }
  }

  else if(SYSCALL_NUM == SYS_SEEK) {
    vaddr_check(f->esp+8);

    int fd = *(uint32_t *)(f->esp+4);
    unsigned ofs = *(uint32_t *)(f->esp+8);
    struct file *file = process_get_file(fd);
    if(file == NULL) exit(-1);

    file_seek(file, ofs);
  }

  else if(SYSCALL_NUM == SYS_TELL) {
    vaddr_check(f->esp+4);

    int fd = *(uint32_t *)(f->esp+4);
    struct file *file = process_get_file(fd);
    if(file == NULL) exit(-1);

    f->eax = file_tell(file);
  }

  else if(SYSCALL_NUM == SYS_CLOSE) {
    vaddr_check(f->esp+4);

    int fd = *(uint32_t *)(f->esp+4);
    bool success = process_close_file(fd);
    if(!success) exit(-1);
  }

  else if(SYSCALL_NUM == SYS_PIBONACCI){
      
    n = (int)*(uint32_t*)(f->esp+4);
    
    f->eax = pibonacci(n);
  }

  else if(SYSCALL_NUM == SYS_SUM_OF_FOUR_INTEGERS){
    
    n1 = (int)*(uint32_t*)(f->esp+4);
    n2 = (int)*(uint32_t*)(f->esp+8);
    n3 = (int)*(uint32_t*)(f->esp+12);
    n4 = (int)*(uint32_t*)(f->esp+16);

    f->eax = sum_of_four_integers(n1,n2,n3,n4);
  }

}

void halt(void){
    shutdown_power_off();
}

void exit(int status){//parent, child 구분해서 구현해야됌

  thread_exit(status);
}

tid_t exec(const char *cmd_line){

  return process_execute(cmd_line);

}

int wait(tid_t pid){

  return process_wait(pid);
}

int pibonacci(int n){

  int a = 1, b = 1, c = 0;
  int i;

  if(n < 0 || n == 1 || n == 2) return n;

  else{

    for(i = 0; i < n-1; i++){
      c = a + b;
      a = b;
      b = c;
    }

  }

  return a;

}

int sum_of_four_integers(int a, int b, int c, int d){
  return (a + b + c + d);
}
