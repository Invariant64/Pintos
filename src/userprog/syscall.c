#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

static void syscall_handler (struct intr_frame *);

static void sys_exit (int);
static int sys_write (int, const void*, unsigned);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{  
  int *addr;
  int ret;

  addr = f->esp;
  if (!is_user_vaddr (addr))
    sys_exit (-1);

  switch (*addr)
    {
      case SYS_EXIT:
        sys_exit (*(addr + 1));
        break;
      case SYS_WRITE:
        ret = sys_write (*(addr + 1), *(addr + 2), *(addr + 3));
        break;
      default:
        sys_exit (-1);
    }
  
  f->eax = ret;
}

void sys_exit (int status)
{ 
  thread_current ()->exit_status = status;
  thread_exit ();
}

int sys_write (int fd, const void *buffer, unsigned size)
{
  if (fd == STDOUT_FILENO)
    putbuf (buffer, size);
  return size;
}
