#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include <string.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"
#include "threads/vaddr.h"
#include "userprog/process.h"
#include "devices/input.h"
#include "filesys/filesys.h"
#include "filesys/file.h"

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
  int _fd;
  int n,n1,n2,n3,n4;
  const void* _buf;
  unsigned _size, _pos;
  const char* _cline;

  vaddr_check(f->esp);
  SYSCALL_NUM = *(int*)(f->esp);

 /* printf("\nsyscall : %d %05x\n\n", SYSCALL_NUM, f->esp);
  hex_dump(f->esp, f->esp, 200, 1);
 
  printf("%d\n",*(uint32_t*)(f->esp+4));
  printf("%d\n",*(uint32_t*)(f->esp+8));
  printf("%d\n",*(uint32_t*)(f->esp+12));
  printf("%d\n\n",*(uint32_t*)(f->esp+16));*/
 

 
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
    _fd = (int)*(uint32_t*)(f->esp+4);
    _buf = (void*)*(uint32_t*)(f->esp+8);
    _size = (unsigned)*(uint32_t*)(f->esp+12);

    f->eax = read(_fd, _buf, _size);
  }

  else if(SYSCALL_NUM == SYS_WRITE){
    
    vaddr_check(f->esp+4);
    vaddr_check(f->esp+8);
    vaddr_check(f->esp+12);
    _fd = (int)*(uint32_t*)(f->esp+4);
    _buf = (const void*)*(uint32_t*)(f->esp+8);
    _size = (unsigned)*(uint32_t*)(f->esp+12);

    f->eax = write(_fd, _buf, _size);
  }

  else if(SYSCALL_NUM == SYS_PIBONACCI){
      
    n = (int)*(uint32_t*)(f->esp + 4);
    
    f->eax = pibonacci(n);
  }

  else if(SYSCALL_NUM == SYS_SUM_OF_FOUR_INTEGERS){
    
    n1 = (int)*(uint32_t*)(f->esp+4);
    n2 = (int)*(uint32_t*)(f->esp+8);
    n3 = (int)*(uint32_t*)(f->esp+12);
    n4 = (int)*(uint32_t*)(f->esp+16);

    f->eax = sum_of_four_integers(n1,n2,n3,n4);
  }

  else if(SYSCALL_NUM == SYS_CREATE){

    vaddr_check(f->esp + 4);
    vaddr_check(f->esp + 8);
    _cline = (const char*)*(uint32_t*)(f->esp + 4);
    _size = (unsigned)*(uint32_t*)(f->esp + 8);

    f->eax = create(_cline,_size);

  }

  else if(SYSCALL_NUM == SYS_REMOVE){

    vaddr_check(f->esp + 4);
    _cline = (const char*)*(uint32_t*)(f->esp + 4);
   
    f->eax = remove(_cline);
  }

  else if(SYSCALL_NUM == SYS_OPEN){

    vaddr_check(f->esp + 4);
    _cline = (const char*)*(uint32_t*)(f->esp + 4);

    f->eax = open(_cline);
  }

  else if(SYSCALL_NUM == SYS_CLOSE){

    vaddr_check(f->esp + 4);
    _fd = (int)*(uint32_t*)(f->esp + 4);

    close(_fd);

  }

  else if(SYSCALL_NUM == SYS_FILESIZE){

    vaddr_check(f->esp + 4);

    _fd = (int)*(uint32_t*)(f->esp + 4);

    f->eax = filesize(_fd);
  }

  else if(SYSCALL_NUM == SYS_SEEK){
   
    vaddr_check(f->esp + 4);
    vaddr_check(f->esp + 8);

    _fd = (int)*(uint32_t*)(f->esp + 4);
    _pos = (unsigned)*(uint32_t*)(f->esp + 8);

    seek(_fd, _pos);
  }

  else if(SYSCALL_NUM == SYS_TELL){

    vaddr_check(f->esp + 4);

    _fd = (int)*(uint32_t*)(f->esp + 4);

    f->eax = tell(_fd);
  }

  //else
  // thread_exit();
  
}

bool create(const char *file, unsigned initial_size){

  if(!file)
    exit(-1);

  return filesys_create(file,initial_size);

}
bool remove(const char *file){

  if(!file)
    exit(-1);

  return filesys_remove(file);

}
int open(const char *file){

  //fd 0 : STDIN, fd 1 : STDOUT

  struct thread *cur;
  struct file *fp_open;

  if(!file) return -1;
  fp_open = filesys_open(file);
  if(!(fp_open)) return -1;

  cur = thread_current();
  int fd_num = cur->fd_limit;

  if(fd_num <= 128){
    if(!strcmp(cur->name, file)){
      file_deny_write(fp_open);
      //fp_open->deny_write = true;
    }
    cur->FD[fd_num] = fp_open;
    cur->fd_limit = cur->fd_limit + 1;

    return fd_num;
  }

  else return -1;
}

void close(int fd){

    struct thread *cur;
    struct file *fp_close;

    cur = thread_current();
    fp_close = cur->FD[fd];

    if(!fp_close) exit(-1);

    file_close(fp_close);
    cur->FD[fd] = NULL;
    fp_close = NULL;
}

int filesize(int fd){

  struct thread *cur;
  cur = thread_current();
  if(!(cur->FD[fd])) exit(-1);

  return file_length(cur->FD[fd]);

}
void seek(int fd, unsigned position){

  struct thread *cur;
  cur = thread_current();
  if(!(cur->FD[fd])) exit(-1);
  file_seek(cur->FD[fd], position);

}
unsigned tell(int fd){

  struct thread *cur;
  cur = thread_current();
  if(!(cur->FD[fd])) exit(-1);

  return file_tell(cur->FD[fd]);

}

void vaddr_check(void* addr){// check valid address

  if(!is_user_vaddr(addr)) exit(-1);

}

void halt(void){
    shutdown_power_off();
}

void exit(int status){//parent, child 구분해서 구현해야됌

  int i;
  struct thread *cur = thread_current();

  for(i = 3; i < 128; i++){
    if(cur->FD[i] != NULL){
      file_close(cur->FD[i]);
      cur->FD[i] = NULL;
    }
  }

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
  struct file *fp;
  struct thread *cur = thread_current();

  if(buffer >= PHYS_BASE) exit(-1);

  if(!fd){
    for(i = 0; i < size; i++){

      *(uint8_t*)(buffer+i) = input_getc();
      
      if(*(uint8_t*)(buffer+i) == '\0') return i;

    }

    return size;
  }

  else{
    if(fd > 128 || fd < 0 || fd == 1 || fd == 2)  return -1;
    else if(cur->FD[fd] == NULL) return -1;
    else{
      fp = cur->FD[fd];
      return file_read(fp,buffer,size);
    }
  }
}

int write(int fd, const void* buffer, unsigned size){
  
    struct file *fp;
    struct thread *cur = thread_current();

    if(fd == 1){
      putbuf(buffer,size);
      return size;
    }

    else{
      if(fd > 128 || fd < 0 || fd == 0 || fd == 2) return -1;
      else if(cur->FD[fd] == NULL) return -1;
      else{
        fp = cur->FD[fd];
        if(fp->deny_write == true)
            file_deny_write(fp);
        else
            file_allow_write(fp);
        return file_write(fp,buffer,size);
      }
   }  
}

int pibonacci(int n){

  int a = 1, b = 1,c = 0;
  int i;

  if( n < 0 || n == 0 || n == 1) return n;

  else{
    for(i = 0; i < n-1 ; i++){
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
