#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

// Define a maximum number of command line arguments
#define MAX_NUM_ARGS 5

struct lock *filesys_lock;
void syscall_init (void);

#endif /* userprog/syscall.h */
