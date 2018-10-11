#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"
#include "threads/vaddr.h"
#include "userprog/process.h"
#include "devices/input.h"

static void syscall_handler (struct intr_frame *);
void vaddr_check(void*);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{ //halt, exit, exec, wait, read, write, fibonacci, sum of 4 integers
  
//  printf ("system call!\n");
 
  
  int SYSCALL_NUM;
  int _fd, n,n1,n2,n3,n4;
  const void* _buf;
  unsigned _size;
  const char* _cline;

  vaddr_check(f->esp);
  SYSCALL_NUM = *(int*)(f->esp);

  //printf("\nsyscall : %d %05x\n", SYSCALL_NUM, f->esp);
 // hex_dump(f->esp, f->esp, 100, 1);

 
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

  else if(SYSCALL_NUM == SYS_READ){

    vaddr_check(f->esp+4);
    vaddr_check(f->esp+8);
    vaddr_check(f->esp+12);
    _fd = *(uint32_t*)(f->esp+4);
    _buf = *(uint32_t*)(f->esp+8);
    _size = *(uint32_t*)(f->esp+12);

    f->eax = read(_fd, _buf, _size);
  }

  else if(SYSCALL_NUM == SYS_WRITE){
    
    vaddr_check(f->esp+4);
    vaddr_check(f->esp+8);
    vaddr_check(f->esp+12);
    _fd = *(uint32_t*)(f->esp+4);
    _buf = *(uint32_t*)(f->esp+8);
    _size = *(uint32_t*)(f->esp+12);

    f->eax = write(_fd, _buf, _size);
  }

  else if(SYSCALL_NUM == SYS_PIBONACCI){
      
    n = (int)*(uint32_t*)(f->esp+8);
    
    f->eax = pibonacci(n);
  }

  else if(SYSCALL_NUM == SYS_SUM_OF_FOUR_INTEGERS){
    
    n1 = (int)*(uint32_t*)(f->esp+8);
    n2 = (int)*(uint32_t*)(f->esp+12);
    n3 = (int)*(uint32_t*)(f->esp+16);
    n4 = (int)*(uint32_t*)(f->esp+20);

    f->eax = sum_of_four_integers(n1,n2,n3,n4);
  }

  //thread_exit ();
  
}

void vaddr_check(void* addr){// check valid address

  if(!is_user_vaddr(addr)) exit(-1);

}

void halt(void){
    shutdown_power_off();
}

void exit(int status){//parent, child 구분해서 구현해야됌

  thread_exit(status);
}

pid_t exec(const char *cmd_line){

  return process_execute(cmd_line);

}

int wait(pid_t pid){

  return process_wait(pid);
}

int read(int fd, void *buffer, unsigned size){

  unsigned int i;

  if(!fd){
    for(i = 0; i < size; i++){

      *(uint8_t*)(buffer+i) = input_getc();
      
      if(*(uint8_t*)(buffer+i) == '\0') return i;

    }

    return size;
  }

  else return -1;
}

int write(int fd, const void* buffer, unsigned size){
  
    if(fd == 1){
      putbuf(buffer,size);
      return size;
    }

    else return -1;
}

int pibonacci(int n){

  if( n==0 || n == 1) return n;

  else return pibonacci(n-1) + pibonacci(n-2);

}

int sum_of_four_integers(int a, int b, int c, int d){
  return (a + b + c + d);
}
