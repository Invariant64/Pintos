#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "filesys/filesys.h"
#include "filesys/file.h"

#define SYS_CALL_NUM 20

typedef int pid_t;

typedef int syscall_func (void *, void *, void *);

static void syscall_handler (struct intr_frame *);

static void     sys_halt     (void);
static void     sys_exit     (int);
static pid_t    sys_exec     (const char*);
static int      sys_wait     (pid_t);
static bool     sys_create   (const char *, unsigned);
static bool     sys_remove   (const char *);
static int      sys_open     (const char *);
static int      sys_filesize (int);
static int      sys_read     (int, void *, unsigned);
static int      sys_write    (int, const void *, unsigned);
static void     sys_seek     (int, unsigned);
static unsigned sys_tell     (int);
static void     sys_close    (int);

syscall_func *sys_func[SYS_CALL_NUM];
int sys_func_argc[SYS_CALL_NUM];

static struct semaphore file_access;

static struct fd* process_get_file (struct process *, int);

static int get_user (const uint8_t *);
static bool user_addr_valid (void *, uint8_t);
static bool check_args_valid (int, void *);

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

  sys_func_argc[SYS_HALT]     = 0;
  sys_func_argc[SYS_EXIT]     = 1;
  sys_func_argc[SYS_EXEC]     = 1;
  sys_func_argc[SYS_WAIT]     = 1;
  sys_func_argc[SYS_CREATE]   = 2;
  sys_func_argc[SYS_REMOVE]   = 1;
  sys_func_argc[SYS_OPEN]     = 1;
  sys_func_argc[SYS_FILESIZE] = 1;
  sys_func_argc[SYS_READ]     = 3;
  sys_func_argc[SYS_WRITE]    = 3;
  sys_func_argc[SYS_SEEK]     = 2;
  sys_func_argc[SYS_TELL]     = 1;
  sys_func_argc[SYS_CLOSE]    = 1;

  sema_init (&file_access, 1);
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{  
  int *addr;
  int number, ret;

  addr = f->esp;
  if (!user_addr_valid (addr, 4))
    sys_exit (-1);

  number = *addr;
  if (number < SYS_HALT || number > SYS_CLOSE)
    sys_exit (-1);

  if (!check_args_valid (sys_func_argc[number], addr))
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
  thread_current ()->process->exit_status = status;
  thread_exit ();
}

pid_t sys_exec (const char *cmd_line)
{
  pid_t pid;

  if (cmd_line == NULL || !user_addr_valid (cmd_line, 4))
    sys_exit (-1);

  sema_down (&file_access);
  
  pid = process_execute (cmd_line);

  sema_up (&file_access);

  return pid;
}

int sys_wait (pid_t pid)
{
  int status;
  
  status = process_wait (pid);

  return status;
}

bool sys_create (const char *file, unsigned initial_size)
{
  bool success;

  if (file == NULL || !user_addr_valid (file, 1))
    sys_exit (-1);

  sema_down (&file_access);

  success = filesys_create (file, initial_size);

  sema_up (&file_access);

  return success;
}

bool sys_remove (const char *file)
{
  bool success;

  if (file == NULL || !user_addr_valid (file, 1))
    sys_exit (-1);

  sema_down (&file_access);

  success = filesys_remove (file);

  sema_up (&file_access);

  return success;
}

int sys_open (const char *file)
{ 
  struct file *f;
  struct fd *fd; 
  struct process *p;
  int fd_ = -1;

  if (file == NULL || !user_addr_valid (file, 4))
    sys_exit (-1);

  sema_down (&file_access);

  p = thread_current ()->process;

  f = filesys_open (file);
  if (f != NULL)
    {
      fd = malloc (sizeof (struct fd));
      fd->fd = fd_ = p->max_fd++;
      fd->file = f;
      list_push_front (&p->files, &fd->elem);
    }

  sema_up (&file_access);

  if (fd_ == NULL)
    sys_exit (-1);

  return fd_;
}

int sys_filesize (int fd)
{
  struct fd *fd_;
  int length;

  if (fd < 2)
    sys_exit (-1);

  sema_down (&file_access);

  fd_ = process_get_file (thread_current ()->process, fd);
  if (fd_ != NULL)
    length = file_length (fd_->file);

  sema_up (&file_access);

  if (fd_ == NULL)
    sys_exit (-1);

  return length;
}

int sys_read (int fd, void *buffer, unsigned size)
{
  struct fd *fd_;
  int size_;

  if (fd < 0 || fd == STDOUT_FILENO || buffer == NULL)
    sys_exit (-1);

  if (!user_addr_valid (buffer, size))
    sys_exit (-1);

  sema_down (&file_access);

  if (fd == STDIN_FILENO)
    {
      for (int i = 0; i < size; i++)
        *((char*) buffer + i) = input_getc ();

      sema_up (&file_access);
      
      return size;
    }

  fd_ = process_get_file (thread_current ()->process, fd);
  if (fd_ != NULL)
    size_ = file_read (fd_->file, buffer, size);

  sema_up (&file_access);

  if (fd_ == NULL)
    sys_exit (-1);

  return size_;
}

int sys_write (int fd, const void *buffer, unsigned size)
{
  struct fd *fd_;
  int size_;

  if (fd < 1 || buffer == NULL)
    sys_exit (-1);

  if (!user_addr_valid (buffer, size))
    sys_exit (-1);

  if (fd == STDOUT_FILENO)
    {
      putbuf (buffer, size);
      return size;
    }
    
  sema_down (&file_access);

  fd_ = process_get_file (thread_current ()->process, fd);
  if (fd_ != NULL)
    size_ = file_write (fd_->file, buffer, size);

  sema_up (&file_access);

  if (fd_ == NULL)
    sys_exit (-1);

  return size_;
}

void sys_seek (int fd, unsigned position)
{
  struct fd *fd_;

  if (fd < 2)
    sys_exit (-1);
  
  sema_down (&file_access);

  fd_ = process_get_file (thread_current ()->process, fd);
  if (fd_ != NULL)
    file_seek (fd_->file, position);

  sema_up (&file_access);

  if (fd_ == NULL)
    sys_exit (-1);
}

unsigned sys_tell (int fd)
{
  struct fd *fd_;
  unsigned position;

  if (fd < 2)
    sys_exit (-1);

  sema_down (&file_access);

  fd_ = process_get_file (thread_current ()->process, fd);
  if (fd_ != NULL)
    position = file_tell (fd_->file);

  sema_up (&file_access);

  if (fd_ == NULL)
    sys_exit (-1);

  return position;
}

void sys_close (int fd)
{
  struct fd *fd_;

  if (fd < 2)
    sys_exit (-1);

  sema_down (&file_access);

  fd_ = process_get_file (thread_current ()->process, fd);
  if (fd_ != NULL)
    {
      list_remove (&fd_->elem);
      file_close (fd_->file);
      free (fd_);
    }

  sema_up (&file_access);

  if (fd_ == NULL)
    sys_exit (-1);
}

struct fd* process_get_file (struct process *p, int fd)
{
  for (struct list_elem *e = list_begin (&p->files);
       e != list_end (&p->files); e = list_next (e))
    {
      struct fd *fd_ = list_entry (e, struct fd, elem);
      if (fd_->fd == fd)
        return fd_;
    }
  return NULL;
}

bool user_addr_valid (void *vaddr, uint8_t length)
{
  uint32_t *pd = thread_current ()->pagedir;
  if (vaddr == NULL || !is_user_vaddr (vaddr) || pagedir_get_page (pd, vaddr) == NULL)
    return false;
  
  for (uint8_t i = 0; i < length; i++)
    if (get_user ((uint8_t*) vaddr + i) == -1)
      return false;

  return true;
}

bool check_args_valid (int argc, void *base_addr)
{
  int *addr = (int*) base_addr;
  for (int i = 1; i <= argc; i++)
    if (!user_addr_valid (addr + i, 4))
      return false;
  return true;
}

/* Reads a byte at user virtual address UADDR.
   UADDR must be below PHYS_BASE.
   Returns the byte value if successful, -1 if a segfault
   occurred. */
int
get_user (const uint8_t *uaddr)
{
  int result;
  asm ("movl $1f, %0; movzbl %1, %0; 1:"
       : "=&a" (result) : "m" (*uaddr));
  return result;
}
