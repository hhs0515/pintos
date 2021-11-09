#ifndef THREADS_THREAD_H
#define THREADS_THREAD_H

#include <debug.h>
#include <list.h>
#include <stdint.h>
/* --------- project_2_parent_child_hierarchy ---------- */
#include "threads/synch.h"

/* States in a thread's life cycle. */
enum thread_status
{
  THREAD_RUNNING, /* Running thread. */
  THREAD_READY,   /* Not running but ready to run. */
  THREAD_BLOCKED, /* Waiting for an event to trigger. */
  THREAD_DYING    /* About to be destroyed. */
};

/* Thread identifier type.
   You can redefine this to whatever type you like. */
typedef int tid_t;
#define TID_ERROR ((tid_t)-1) /* Error value for tid_t. */

/* Thread priorities. */
#define PRI_MIN 0      /* Lowest priority. */
#define PRI_DEFAULT 31 /* Default priority. */
#define PRI_MAX 63     /* Highest priority. */
/* Priority donation */
#define MAX_NESTED_DEPTH 8 // [EDITED_project_1_prioirty_donation]
/* MLFQS */
#define NICE_DEFAULT 0       // [EDITED_project_1_MLFQS]
#define RECENT_CPU_DEFAULT 0 // [EDITED_project_1_MLFQS]
#define LOAD_AVG_DEFAULT 0   // [EDITED_project_1_MLFQS]
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
  tid_t tid;                 /* Thread identifier. */
  enum thread_status status; /* Thread state. */
  char name[16];             /* Name (for debugging purposes). */
  uint8_t *stack;            /* Saved stack pointer. */
  int priority;              /* Priority. */
  struct list_elem allelem;  /* List element for all threads list. */

  /* --------- project_1 ---------- */
  // Alarm_System_Call
  int64_t wait_until; /* save tick when to wake up */
  // Prioirity_Scheduling
  int prev_priority;              // to save the previous priority before donation
  struct lock *waiting_lock;      // the lock pointer which this thread waits for this lock realesing.
  struct list donator_list;       // the list of donators for this thread's priority, used at multiple donation
  struct list_elem donation_elem; // used when this thread donate to others
  // MLFQS
  int nice;
  int recent_cpu;
  /* -------------------------------------- */
  /* --------- project_2_parent_child_hierarchy ---------- */
  struct semaphore load_sema;
  struct semaphore exit_sema;
  struct thread *parent_thread;
  struct list child_list;
  struct list_elem child_elem;
  bool load_done;
  bool exit_done;
  int exit_status;

  /* --------- project_2_file_descriptor ---------- */
  struct file **file_descriptor;
  int cur_fd;
  char **fd_name;

  /* --------- project_2_denying_writes_to_executables ---------- */
  struct file *executable;

  /* Shared between thread.c and synch.c. */
  struct list_elem elem; /* List element. */

#ifdef USERPROG
  /* Owned by userprog/process.c. */
  uint32_t *pagedir; /* Page directory. */
#endif

  /* Owned by thread.c. */
  unsigned magic; /* Detects stack overflow. */
};

/* If false (default), use round-robin scheduler.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "-o mlfqs". */
extern bool thread_mlfqs;

/* ----------------- project_1 ADDED ----------------- */

// Alarm Clock
/* getter & setter for first_awake_tick (alarm clock) */
void set_first_awake_tick(int16_t ticks);
int64_t get_first_awake_tick(void);

/* sleep thread for ticks time */
void thread_sleep(int64_t ticks);
void thread_awake(int64_t ticks);

// Priority Scheduling
bool compare_thread_priority(const struct list_elem *p, const struct list_elem *q, void *aux UNUSED);
void compare_curThread_readyList_priority(void);

// Priority Inversion(donation)
void donate_priority(void); // for nested donation
void remove_lock_donator(struct lock *lock);
void update_priority(void);

// MLFQS
void MLFQS_cal_priority(struct thread *t);   // calculate priority
void MLFQS_cal_recent_cpu(struct thread *t); // calculate recent_cpu
void MLFQS_cal_load_avg(void);               // calculate load_avg
void MLFQS_incre_recent_cpu(void);           // increment recent_cpu 1 at every 1 tick.
void MLFQS_recal_priority(void);             // recalculate entire thread's prority at every 4 ticks
void MLFQS_recal_recent_cpu(void);           // recalculate entire thread's recent_cpu at every 1 second

/* --------------------------------------------------- */

void thread_init(void);
void thread_start(void);

void thread_tick(void);
void thread_print_stats(void);

typedef void thread_func(void *aux);
tid_t thread_create(const char *name, int priority, thread_func *, void *);

void thread_block(void);
void thread_unblock(struct thread *);

struct thread *thread_current(void);
tid_t thread_tid(void);
const char *thread_name(void);

void thread_exit(void) NO_RETURN;
void thread_yield(void);

/* Performs some operation on thread t, given auxiliary data AUX. */
typedef void thread_action_func(struct thread *t, void *aux);
void thread_foreach(thread_action_func *, void *);

int thread_get_priority(void);
void thread_set_priority(int);

int thread_get_nice(void);
void thread_set_nice(int);
int thread_get_recent_cpu(void);
int thread_get_load_avg(void);

#endif /* threads/thread.h */
