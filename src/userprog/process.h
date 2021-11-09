#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"
#include <string.h>

tid_t process_execute(const char *file_name);
int process_wait(tid_t);
void process_exit(void);
void process_activate(void);
/* [EDITED_Lab2_argument_passing] */
void save_stack(char **argv_item_list, int argc, void **esp);
int tokenize(char **token_list, char *file_name);
/* [EDITED_Lab2_parent_child_process] */
struct thread *get_child(int pid);
void *remove_child(struct thread *p_cur);
/* [EDITED_Lab2_file_descriptor] */
int add_to_fd (struct file * f, char *filename);
struct file * get_file(int fd);
void close_file(int fd);

#endif /* userprog/process.h */
