#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"

// 4 Kilobytes is the size limit for the command line arguments
#define ARGS_MAX_SIZE 4000

// Limit the length of each command line argument to 50 characters
#define ARG_MAX_LEN 50

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

#endif /* userprog/process.h */
