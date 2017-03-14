#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"

static void syscall_handler (struct intr_frame *);
static bool is_valid_addr (const void *intr_addr);
static void constr_args (struct intr_frame *f, int argc, int *argv[]);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
	const void *intr_addr;
	int *argv[MAX_NUM_ARGS];
	int scn;

	// Terminate user process if given address is not valid
	intr_addr = (const void *)f;
	if (!is_valid_addr (intr_addr))
		thread_exit ();

	// Get the System Call Number from the given interrupt frame (vaddr.h)
	scn = *(int*) f->esp; 

	// Call the corresponding handler for the given System Call Number
	switch (scn)
	{
		case SYS_HALT:
			shutdown_power_off ();
		case SYS_EXIT:
			constr_args (f,  1, argv);
			exit (argv[0]);
			break;
		case SYS_EXEC:
			break;
		case SYS_WAIT:
			break;
		case SYS_CREATE:
			break;
		case SYS_REMOVE:
			break;
		case SYS_OPEN:
			break;
		case SYS_FILESIZE:
			break;	
		case SYS_READ:
			break;
		case SYS_WRITE:
			break;
		case SYS_SEEK:
			break;
		case SYS_TELL:
			break;
		case SYS_CLOSE:
			break;
		default:
			thread_exit ();
	}
}

static bool
is_valid_addr (const void *addr)
{
	bool is_valid = true;

	// Validate that the given address is in the user virtual address space
	if (is_user_vaddr (addr))
		is_valid = false;

	// Validate that the given address is not also partially in kernel address space	
	if (is_kernel_vaddr (addr))
		is_valid = false;

	return is_valid;
}

// const struct intr_frame *f: 		the address of the interrupt frame
// int argc: 											the number of arguments within the interrupt frame
// char *argv[]: 								  an empty array that will be loaded with the arguments
static void
constr_args (struct intr_frame *f, int argc, int *argv[])
{
	int *esp;
	int i;
	
	// Get a handle to the interrupt frame's stack pointer holding the arguments
	esp = (int*) f->esp;	

	// Extract the given number of arguments, using the stack pointer as reference
	for (i = 1; i < argc; i++)
	{
		// Get the address of the next argument - offset by 1 for the system call number at arg[0]	
		esp++;
		argv[i] = esp;

		// Validate that the argument address is a valid user address
		if (!is_user_vaddr((const void *) argv[i]))
			thread_exit ();
	}

	// Set a null pointer to mark the end of the command line arguments
	argv[argc] = NULL;
}

