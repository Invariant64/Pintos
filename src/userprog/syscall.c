#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

#define SYS_CALL_NUM 20

typedef int pid_t;

typedef int syscall_func (void *param1, void *param2, void *param3);

static void syscall_handler (struct intr_frame *);

static void     sys_halt     (void);
static void     sys_exit     (int);
static pid_t    sys_exec     (const char *);
static int      sys_wait     (pid_t);
static bool     sys_create   (const char *file, unsigned initial_size);
static bool     sys_remove   (const char *file);
static int      sys_open     (const char *file);
static int      sys_filesize (int fd);
static int      sys_read     (int fd, void *buffer, unsigned size);
static int      sys_write    (int, const void*, unsigned);
static void     sys_seek     (int fd, unsigned position);
static unsigned sys_tell     (int fd);
static void     sys_close    (int fd);

syscall_func *sys_func[SYS_CALL_NUM];

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");

  sys_func[SYS_HALT]     = sys_halt;
  sys_func[SYS_EXIT]     = sys_exit;
  sys_func[SYS_EXEC]     = sys_exec;  
  sys_func[SYS_WAIT]     = sys_wait;
  sys_func[SYS_CREATE]   = sys_create;
  sys_func[SYS_REMOVE]   = sys_remove;
  sys_func[SYS_OPEN]     = sys_open;
  sys_func[SYS_FILESIZE] = sys_filesize;  
  sys_func[SYS_READ]     = sys_read;
  sys_func[SYS_WRITE]    = sys_write;
  sys_func[SYS_SEEK]     = sys_seek;
  sys_func[SYS_TELL]     = sys_tell;
  sys_func[SYS_CLOSE]    = sys_close; 
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{  
  int *addr;
  int number, ret;

  addr = f->esp;
  if (!is_user_vaddr (addr))
    sys_exit (-1);

  number = *addr;
  if (number < SYS_HALT || number > SYS_CLOSE)
    sys_exit (-1);
  
  ret = sys_func[number] (*(addr + 1), *(addr + 2), *(addr + 3));

  f->eax = ret;
}

void sys_halt (void)
{
  shutdown_power_off ();
}

void sys_exit (int status)
{ 
  thread_current ()->exit_status = status;
  thread_exit ();
}

pid_t sys_exec (const char *cmd_line)
{

}

int sys_wait (pid_t pid)
{

}

bool sys_create (const char *file, unsigned initial_size)
{

}

bool sys_remove (const char *file)
{

}
int sys_open (const char *file)
{

}

int sys_filesize (int fd)
{

}

int sys_read (int fd, void *buffer, unsigned size)
{

}

int sys_write (int fd, const void *buffer, unsigned size)
{
  if (fd == STDOUT_FILENO)
    putbuf (buffer, size);
  return size;
}

void sys_seek (int fd, unsigned position)
{

}

unsigned sys_tell (int fd)
{

}

void sys_close (int fd)
{

}