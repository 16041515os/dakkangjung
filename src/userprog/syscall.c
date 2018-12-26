#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include "userprog/process.h"
#include "userprog/pagedir.h"
#include "filesys/file.h"
#include "filesys/filesys.h"


#ifdef VM
#include "vm/page.h"
#include "vm/frame.h"
static void force_load_and_pin(uint8_t *, size_t);
static void unpin_loaded_pages(uint8_t *, size_t);
#endif

static struct lock filesys_lock;

static int get_user (const uint8_t *uaddr);
static bool put_user (uint8_t *udst, uint8_t byte);
static size_t read_bytes (void *dest, void *src, size_t nb);

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  lock_init(&filesys_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{ 
  thread_current()->user_esp = f->esp;
  
  int SYSCALL_NUM;

  read_bytes(&SYSCALL_NUM, f->esp, sizeof SYSCALL_NUM);

  if(SYSCALL_NUM == SYS_HALT){
      halt();
  }

  else if(SYSCALL_NUM == SYS_EXIT){
    int exit_code;
    read_bytes(&exit_code, f->esp + 4, sizeof exit_code);

    exit(exit_code);
  }

  else if(SYSCALL_NUM == SYS_EXEC){
    char *cmdline;
    char dummy;
    read_bytes(&cmdline, f->esp + 4, sizeof cmdline);
    read_bytes(&dummy, cmdline, sizeof dummy);

    f->eax = exec((char const *)cmdline);
  }

  else if(SYSCALL_NUM == SYS_WAIT){
    tid_t pid;
    read_bytes(&pid, f->esp + 4, sizeof pid);
    f->eax = wait(pid);
  }

  else if(SYSCALL_NUM == SYS_CREATE) {
    char *fname;
    char dummy;
    unsigned size;

    read_bytes(&fname, f->esp + 4, sizeof fname);
    read_bytes(&dummy, fname, sizeof dummy);
    read_bytes(&size, f->esp + 8, sizeof size);

    f->eax = filesys_create((char const *)fname, size);
  }

  else if(SYSCALL_NUM == SYS_REMOVE) {
    char *fname;
    char dummy;

    read_bytes(&fname, f->esp + 4, sizeof fname);
    read_bytes(&dummy, fname, sizeof dummy);

    f->eax = filesys_remove((char const *)fname);
  }

  else if(SYSCALL_NUM == SYS_OPEN) {
    char *fname;
    char dummy;

    read_bytes(&fname, f->esp + 4, sizeof fname);
    read_bytes(&dummy, fname, sizeof dummy);

    f->eax = process_open_file((char const *)fname);
  }

  else if(SYSCALL_NUM == SYS_FILESIZE) {
    int fd;
    struct file *file;

    read_bytes(&fd, f->esp + 4, sizeof fd);

    file = process_get_file(fd);
    if(file == NULL) thread_exit(-1);

    f->eax = file_length(file);
  }

  else if(SYSCALL_NUM == SYS_READ){
    int fd;
    uint8_t *buffer;
    size_t size;

    read_bytes(&fd, f->esp + 4, sizeof fd);
    read_bytes(&buffer, f->esp + 8, sizeof buffer);
    read_bytes(&size, f->esp + 12, sizeof size);

    if(fd == 1 || fd == 2) thread_exit(-1);

    if(size > 0 && (!put_user(buffer, 0x00) || !put_user(buffer + size-1, 0x00))) {
      thread_exit(-1);
    }

    if(fd == 0) {
      unsigned i;
      lock_acquire(&filesys_lock);
      for(i = 0; i < size; i++){
        buffer[i] = input_getc();
      }
      f->eax = size;
    }
    else {
      struct file *file = process_get_file(fd);
      if(file == NULL) thread_exit(-1);

      lock_acquire(&filesys_lock);
#ifdef VM
      force_load_and_pin(buffer, size);
#endif
      f->eax = file_read(file, buffer, size);
#ifdef VM
      unpin_loaded_pages(buffer, size);
#endif
      lock_release(&filesys_lock);
    }
  }

  else if(SYSCALL_NUM == SYS_WRITE){
    int fd;
    uint8_t *buffer;
    size_t size;

    read_bytes(&fd, f->esp + 4, sizeof fd);
    read_bytes(&buffer, f->esp + 8, sizeof buffer);
    read_bytes(&size, f->esp + 12, sizeof size);

    if(fd == 0 || fd == 2) thread_exit(-1);

    if(size > 0 && (get_user(buffer) == -1 || get_user(buffer + size-1) == -1)) {
      thread_exit(-1);
    }

    if(fd == 1 /*|| fd == 2*/) {
      lock_acquire(&filesys_lock);
      putbuf((char *)buffer, size);
      lock_release(&filesys_lock);
      f->eax = size;
    }
    else {
      struct file *file = process_get_file(fd);
      if(file == NULL) thread_exit(-1);

      lock_acquire(&filesys_lock);
#ifdef VM
      force_load_and_pin(buffer, size);
#endif
      f->eax = file_write(file, buffer, size);
#ifdef VM
      unpin_loaded_pages(buffer, size);
#endif
      lock_release(&filesys_lock);
    }
  }

  else if(SYSCALL_NUM == SYS_SEEK) {
    int fd;
    size_t ofs;

    read_bytes(&fd, f->esp + 4, sizeof fd);
    read_bytes(&ofs, f->esp + 8, sizeof ofs);

    struct file *file = process_get_file(fd);
    if(file == NULL) thread_exit(-1);

    file_seek(file, ofs);
  }

  else if(SYSCALL_NUM == SYS_TELL) {
    int fd;
    read_bytes(&fd, f->esp + 4, sizeof fd);

    struct file *file = process_get_file(fd);
    if(file == NULL) thread_exit(-1);

    f->eax = file_tell(file);
  }

  else if(SYSCALL_NUM == SYS_CLOSE) {
    int fd;
    read_bytes(&fd, f->esp + 4, sizeof fd);

    bool success = process_close_file(fd);
    if(!success) thread_exit(-1);
  }

  else if(SYSCALL_NUM == SYS_PIBONACCI) {
    int n;
    read_bytes(&n, f->esp + 4, sizeof n);
    f->eax = pibonacci(n);
  }

  else if(SYSCALL_NUM == SYS_SUM_OF_FOUR_INTEGERS){
    int n1, n2, n3, n4;
    
    read_bytes(&n1, f->esp + 4, sizeof n1);
    read_bytes(&n2, f->esp + 8, sizeof n1);
    read_bytes(&n3, f->esp + 12, sizeof n1);
    read_bytes(&n4, f->esp + 16, sizeof n1);

    f->eax = sum_of_four_integers(n1,n2,n3,n4);
  }
  else {
    // unknown syscall
    thread_exit(-1);
  }

}

 	
/* Reads a byte at user virtual address UADDR.
   UADDR must be below PHYS_BASE.
   Returns the byte value if successful, -1 if a segfault
   occurred. */
static int
get_user (const uint8_t *uaddr)
{
  if(!is_user_vaddr(uaddr)) return -1;

  int result;
  asm ("movl $1f, %0; movzbl %1, %0; 1:"
       : "=&a" (result) : "m" (*uaddr));
  return result;
}
 
/* Writes BYTE to user address UDST.
   UDST must be below PHYS_BASE.
   Returns true if successful, false if a segfault occurred. */
static bool
put_user (uint8_t *udst, uint8_t byte)
{
  if(!is_user_vaddr(udst)) return false;

  int error_code;
  asm ("movl $1f, %0; movb %b2, %1; 1:"
       : "=&a" (error_code), "=m" (*udst) : "q" (byte));
  return error_code != -1;
}

/* == memcpy(dest, src, nb) */
static size_t read_bytes (void *dest, void *src, size_t nb) {
  size_t i;

  ASSERT(is_kernel_vaddr(dest));

  if(get_user(src+0) == -1 || get_user(src+nb-1) == -1) thread_exit(-1);

  for(i=0; i<nb;  ++i) {
    ((uint8_t *)dest)[i] = ((uint8_t *)src)[i];
  }

  return nb;
}

static void force_load_and_pin(uint8_t *buf, size_t size) {
  uint8_t *upage;
  for(upage = pg_round_down(buf); upage < buf + size; upage += PGSIZE) {
    if(pagedir_get_page(thread_current()->pagedir, upage) == NULL) {
      void *kpage = supt_load_page(thread_current(), upage);
      ASSERT(kpage != NULL);
      frame_set_pinned(kpage, true);
    }
  }
}

static void unpin_loaded_pages(uint8_t *buf, size_t size) {
  uint8_t *upage;
  for(upage = pg_round_down(buf); upage < buf + size; upage += PGSIZE) {
    void *kpage = pagedir_get_page(thread_current()->pagedir, upage);
    ASSERT(kpage != NULL);
    frame_set_pinned(kpage, false);
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
