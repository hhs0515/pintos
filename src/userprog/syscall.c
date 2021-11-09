#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
/* [ADDED_Lab2_system_call] */
#include <devices/shutdown.h>
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "userprog/process.h"
#include <string.h>
#include "threads/synch.h"
#include "devices/input.h"
#define PHYS_BASE 0xc0000000;

static void syscall_handler(struct intr_frame *);

void syscall_init(void)
{
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&file_lock);
}

static void
syscall_handler(struct intr_frame *f)
{
  int MAX_ARG = 7;
  int argv[MAX_ARG];
  void *esp = f->esp; // user's stack pointer
  /* check esp is valid user area pointer */
  is_userArea(esp);
  int syscall_code = *(int *)esp;
  //printf("\n kyungphil_syscall_code: %d\n", syscall_code);

  /* sys call handler */
  switch (syscall_code)
  {
    case SYS_HALT:
    {
      shutdown_power_off();
      break;
    }

    case SYS_EXIT:
    {
      read_arg(esp, argv, 1); // read exit status from args
      int exit_status = argv[0];
      sys_exit(exit_status);
      break;
    }

    case SYS_EXEC:
    {
      //printf("\n kyungphil_syscall\n");
      read_arg(esp, argv, 1);
      is_userArea((void *)argv[0]);
      f->eax = -1;
      /* wait for child */
      char *exec_file = (char *)argv[0];
      tid_t pid = process_execute(exec_file);
      struct thread *child = get_child(pid);
      sema_down(&child->load_sema);
      if (child->load_done)
        f->eax = pid;
      break;
    }
    case SYS_WAIT:
    {
      read_arg(esp, argv, 1);
      tid_t pid = argv[0];
      int status = process_wait(pid);
      f->eax = status;
      break;
    }

    case SYS_CREATE:
    {
      read_arg(esp, argv, 2); // read create args
      is_userArea((void *)argv[0]);
      char *filename = (char *)argv[0];
      unsigned size_file = argv[1];
      bool ret = filesys_create(filename, size_file);
      f->eax = ret;
      break;
    }

    case SYS_REMOVE:
    {
      read_arg(esp, argv, 1); // read remove args
      is_userArea((void *)argv[0]);
      char *filename = (char *)argv[0];
      bool ret = filesys_remove(filename);
      f->eax = ret;
      break;
    }

    case SYS_OPEN:
    {
      read_arg(esp, argv, 1); // read open args
      is_userArea((void *)argv[0]);
      char *filename = (char *)argv[0];
      int ret = -1;
      lock_acquire(&file_lock);
      if(filename != NULL)
      {
        struct file *f = filesys_open(filename);
        if(f)
        {
          ret = add_to_fd(f, filename);
        }
      }
      f->eax = ret;
      break;
    }

    case SYS_FILESIZE:
    {
      read_arg(esp, argv, 1); // read filesize args
      is_userArea((void *)argv[0]);
      char *filename = (char *)argv[0];
      int fd, ret;
      fd = fd_lookup(filename);
      struct file *f_read = get_file(fd);
      if(f_read != NULL)
      {
        ret = file_length(f_read);
      }
      f->eax = ret;
      break;
    }

    case SYS_READ:
    {
      read_arg(esp, argv, 3); // read args for read-op
      is_userArea((void *)argv[0]);
      int fd = argv[0];
      void *buffer = (void *)argv[1];
      unsigned size_file = argv[2];
      int ret = read(fd, buffer, size_file);
      f->eax = ret;
      break;
    }

    case SYS_WRITE:
    {
      read_arg(esp, argv, 3); // read args for write-op
      is_userArea((void *)argv[0]);
      int fd = argv[0];
      void *buffer = (void *)argv[1];
      unsigned size_file = argv[2];
      int ret = write(fd, buffer, size_file);
      f->eax = ret;
      break;
    }

    case SYS_SEEK:
    {
      read_arg(esp, argv, 2); // read file args
      is_userArea((void *)argv[0]);
      char *filename = (char *)argv[0];
      unsigned pos = argv[1];
      int fd;
      fd = fd_lookup(filename);
      struct file *f_read = get_file(fd);
      if(f_read != NULL)
      {
        file_seek(f_read, pos);
      }
      break;
    }

    case SYS_TELL:
    {
      read_arg(esp, argv, 1); // read file args
      is_userArea((void *)argv[0]);
      char *filename = (char *)argv[0];
      int fd, ret;
      fd = fd_lookup(filename);
      struct file *f_read = get_file(fd);
      if(f_read != NULL)
      {
        ret = file_tell(f_read);
      }
      f->eax = ret;
      break;
    }

    case SYS_CLOSE:
    {
      read_arg(esp, argv, 1); // read filesize args
      is_userArea((void *)argv[0]);
      char *filename = (char *)argv[0];
      int fd;
      fd = fd_lookup(filename);
      if(fd >= 2)
      {
        close_file(fd);
      }
      break;
    }
  }
}

/* [ADDED_Lab2_system_call] */
void is_userArea(void *uaddr)
{
  // check that a user pointer `uaddr` points below PHYS_BASE
  uint32_t val = (unsigned int)uaddr;
  uint32_t min_address = 0x8048000;
  uint32_t max_address = PHYS_BASE;

  if (val < min_address || val >= max_address)
  {
    sys_exit(-1);
  }
}
/* [ADDED_Lab2_system_call] */
void read_arg(void *esp, int *arg, int num_arg_item)
{
  if (num_arg_item <= 0)
    return;

  int p;
  void *ptr = esp + 4;
  for (p = 0; p < num_arg_item; p++)
  {
    is_userArea(ptr);
    arg[p] = *(int *)ptr;
    ptr += 4;
  }
}
/* [ADDED_Lab2_system_call] */
void sys_exit(int exit_status)
{
  struct thread *t = thread_current();
  t->exit_status = exit_status;
  printf("%s: exit(%d)\n", thread_name(), exit_status);
  thread_exit();
}

int read(int fd, void *buffer, unsigned size)
{
  lock_acquire(&file_lock);
  if (fd == 0)      //STDIN
  {
    int i = size;
    char *buf = (char *) buffer;
    while(i--)
    {
      buf[i] = input_getc();
    }
    lock_release(&file_lock);
    return size;
  }
  else
  {
    struct file *f = get_file(fd);
    if(f == NULL)
    {
      lock_release(&file_lock);
      return -1;
    }
    int i;
    i = file_read(f, buffer, size);
    lock_release(&file_lock);

    return i;
  }
}

int write(int fd, void *buffer, unsigned size)
{
  lock_acquire(&file_lock);
  if(fd == 1)     //STDOUT
  {
    putbuf(buffer, size);
    lock_release(&file_lock);
    return size;
  }
  else
  {
    struct file *f = get_file(fd);
    if(f == NULL)
    {
      lock_release(&file_lock);
      return -1;
    }
    int i;
    i = file_write(f, buffer, size);
    lock_release(&file_lock);

    return i;
  }

}

int fd_lookup(char * filename)  //filename to fd
{
  struct thread *t = thread_current();
  int i = 2;
  for(i = 2; i < 128; i++)
  {
    if (strcmp(filename,t->fd_name[i]) == 0)
    {
      return i;
    }
  }
  return -1;
}
