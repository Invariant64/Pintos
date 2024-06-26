#ifndef THREADS_THREAD_H
#define THREADS_THREAD_H

#include <debug.h>
#include <list.h>
#include <stdint.h>
#include "threads/synch.h"
#include "threads/fixed-point.h"

/* States in a thread's life cycle. */
enum thread_status
  {
    THREAD_RUNNING,     /* Running thread. */
    THREAD_READY,       /* Not running but ready to run. */
    THREAD_BLOCKED,     /* Waiting for an event to trigger. */
    THREAD_DYING        /* About to be destroyed. */
  };

/* Thread identifier type.
   You can redefine this to whatever type you like. */
typedef int tid_t;
#define TID_ERROR ((tid_t) -1)          /* Error value for tid_t. */

/* Thread priorities. */
#define PRI_MIN 0                       /* Lowest priority. */
#define PRI_DEFAULT 31                  /* Default priority. */
#define PRI_MAX 63                      /* Highest priority. */
#define PRI_INVALID -1                  /* Invalied priority. */

#define DONATION_MAX_DEPTH 8            /* Maximum depth of donation. */

#ifdef USERPROG

/* Struct process saves process' infomation, when creating a process,
   malloc new space for process, because if there is process FA and its
   child process A, A ended before FA ended, then A's space was released,
   FA can't get A's termination information. So process need a dependent
   space. 
   
   When creating a process thread, the process and its parent need to 
   save the process pointer. Only when the last one of the process and
   its parent ended, the process' space was released. */
struct process
{
   struct thread *process_thread;      /* Thread that the process belongs to. */
   bool terminated;                    /* Is the process terminated. */
   struct semaphore sema;              /* Semaphore uses to wait for process ends. */
   int exit_status;                    /* Process' exit status. */
   struct list_elem elem;              /* List element. */
   int max_fd;
   struct list files;
   int pid;
   bool waited;
   bool success;
};

struct fd
{
   int fd;
   struct file *file;
   struct list_elem elem;
};
#endif

/* A kernel thread or user process.

   Each thread structure is stored in its own 4 kB page.  The
   thread structure itself sits at the very bottom of the page
   (at offset 0).  The rest of the page is reserved for the
   thread's kernel stack, which grows downward from the top of
   the page (at offset 4 kB).  Here's an illustration:

        4 kB +---------------------------------+
             |          kernel stack           |
             |                |                |
             |                |                |
             |                V                |
             |         grows downward          |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             +---------------------------------+
             |              magic              |
             |                :                |
             |                :                |
             |               name              |
             |              status             |
        0 kB +---------------------------------+

   The upshot of this is twofold:

      1. First, `struct thread' must not be allowed to grow too
         big.  If it does, then there will not be enough room for
         the kernel stack.  Our base `struct thread' is only a
         few bytes in size.  It probably should stay well under 1
         kB.

      2. Second, kernel stacks must not be allowed to grow too
         large.  If a stack overflows, it will corrupt the thread
         state.  Thus, kernel functions should not allocate large
         structures or arrays as non-static local variables.  Use
         dynamic allocation with malloc() or palloc_get_page()
         instead.

   The first symptom of either of these problems will probably be
   an assertion failure in thread_current(), which checks that
   the `magic' member of the running thread's `struct thread' is
   set to THREAD_MAGIC.  Stack overflow will normally change this
   value, triggering the assertion. */
/* The `elem' member has a dual purpose.  It can be an element in
   the run queue (thread.c), or it can be an element in a
   semaphore wait list (synch.c).  It can be used these two ways
   only because they are mutually exclusive: only a thread in the
   ready state is on the run queue, whereas only a thread in the
   blocked state is on a semaphore wait list. */
struct thread
  {
    /* Owned by thread.c. */
    tid_t tid;                          /* Thread identifier. */
    enum thread_status status;          /* Thread state. */
    char name[16];                      /* Name (for debugging purposes). */
    uint8_t *stack;                     /* Saved stack pointer. */
    int priority;                       /* Priority. */
    struct list_elem allelem;           /* List element for all threads list. */

    /* Shared between thread.c and synch.c. */
    struct list_elem elem;              /* List element. */

    int64_t wakeup_ticks;                /* Ticks until thread wakes up. */

    int previous_priority;               /* Previous priority. */
    struct list donations;               /* Donations. */

    struct lock *acquired_lock;          /* Lock acquired by blocked thread. */

    /* Mlfqs. */
    int nice;
    fixed_point recent_cpu;

#ifdef USERPROG
    /* Owned by userprog/process.c. */
    uint32_t *pagedir;                  /* Page directory. */
   
    struct process *parent_process;     /* Parent thread. */      
    struct process *process;            /* Process pointer. */
    struct list children;               /* Child processes. */

    struct semaphore sema;

    struct thread *parent_thread;
    bool success;
#endif

    /* Owned by thread.c. */
    unsigned magic;                     /* Detects stack overflow. */
  };

/* For each thread, one lock can only create one donation,
   the priority of donation is the highest priority of all
   donating threads. */
struct donation
   {
      struct list_elem elem;            /* List element. */

      struct lock *lock;                /* Lock causing the donations. */
      int priority;                     /* Highest priority of all donations of the lock. */
   };

/* If false (default), use round-robin scheduler.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "-o mlfqs". */
extern bool thread_mlfqs;

extern fixed_point load_avg;

extern struct list ready_list;
extern struct list sleep_list;

extern struct list ready_queue[PRI_MAX + 1];

void thread_init (void);
void thread_start (void);

void thread_tick (void);
void thread_wakeup (void);
void thread_print_stats (void);

typedef void thread_func (void *aux);
tid_t thread_create (const char *name, int priority, thread_func *, void *);

void thread_block (void);
void thread_unblock (struct thread *);

struct thread *thread_current (void);
tid_t thread_tid (void);
const char *thread_name (void);

void thread_exit (void) NO_RETURN;
void thread_yield (void);
void thread_try_yield (void);

/* Performs some operation on thread t, given auxiliary data AUX. */
typedef void thread_action_func (struct thread *t, void *aux);
void thread_foreach (thread_action_func *, void *);

int thread_get_priority (void);
void thread_set_priority (int);

int thread_get_nice (void);
void thread_set_nice (int);
int thread_get_recent_cpu (void);
int thread_get_load_avg (void);

void thread_update_priority (struct thread *, void *aux);
void thread_update_recent_cpu (struct thread *, void *aux);
void thread_update_load_avg (void);
void thread_update_all (void);

void thread_insert_ready_queue (struct thread *);
void thread_adjust_queues (void);

int get_max_nempty_queue (void);

void thread_donate (struct thread *, struct thread *, struct lock *);
void thread_release (struct lock *);

bool priority_less (const struct list_elem *a_, 
                    const struct list_elem *b_,
                    void *aux);

bool donation_less (const struct list_elem *a_, 
                    const struct list_elem *b_,
                    void *aux);

bool wakeuptime_less (const struct list_elem *a_, 
                      const struct list_elem *b_,
                      void *aux);

#endif /* threads/thread.h */
