#include <stdio.h>
#include <syscall-nr.h>
#include "lib/user/syscall.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "userprog/syscall.h"
#include "devices/shutdown.h"

static void syscall_handler (struct intr_frame *);
bool is_valid_addr (const void *intr_addr);
void constr_args (struct intr_frame *f, int argc, int *argv[]);

void
syscall_init () 
{ intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
	printf("### Inside syscall.c::syscall_handler ###\n");

	const void *intr_addr;
	int *argv[MAX_NUM_ARGS];
	int argc;
	int scn;

	// Terminate user process if given address is not valid
	intr_addr = (const void *)f;
	if (!is_valid_addr (intr_addr))
		thread_exit ();

	// Get the System Call Number from the given interrupt frame (vaddr.h)
	scn = *(int*) f->esp; 

	// Call the corresponding handler for the given System Call Number
	// f: 	 the interrupt frame with stack pointer and metadata
	// argc: the number of arguments in the frame
	// argv: an empty array given to constr_args which extracts and assembles
	//       an argc number of arguments from f
	switch (scn)
	{
		case SYS_HALT:
			halt ();
		case SYS_EXIT:
			argc = 1;
			constr_args (f,  argc-1, argv);
			exit (argv[0]);
			break;
		case SYS_EXEC:
			argc = 1;
			constr_args (f, argc-1, argv);
			f->eax = exec (argv[0]);	
			break;
		case SYS_WAIT:
			argc = 1;
			constr_args (f, argc-1, argv);
			f->eax = wait (argv[0]);	
			break;
		case SYS_CREATE:
			argc = 2;
			constr_args (f, argc-1, argv);
			f->eax = create ((const char*) argv[0], (unsigned) argv[1]);
			break;
		case SYS_REMOVE:
			argc = 1;
			constr_args (f, argc-1, argv);
			f->eax = remove (argv[0]);	
			break;
		case SYS_OPEN:
			argc = 1;
			constr_args (f, argc-1, argv);
			f->eax = open (argv[0]);
			break;
		case SYS_FILESIZE:
			argc = 1;
			constr_args (f, argc-1, argv);
			f->eax = filesize (argv[0]);
			break;	
		case SYS_READ:
			argc = 3;
			constr_args (f, argc-1, argv);
			f->eax = read (argv[0], argv[1], argv[2]);
			break;
		case SYS_WRITE:
			argc = 3;
			constr_args (f, argc-1, argv);
			f->eax = write (argv[0], argv[1], argv[2]);
			break;
		case SYS_SEEK:
			argc = 2;
			constr_args (f, argc-1, argv);
			seek (argv[0], argv[2]);
			break;
		case SYS_TELL:
			argc = 1;
			constr_args (f, argc-1, argv);
			f->eax = tell (argv[0]);
			break;
		case SYS_CLOSE:
			argc = 1;
			constr_args (f, argc-1, argv);
			close (argv[0]);
			break;
		default:
			thread_exit ();
	}
}

// Validates that the given address is a valid address in the user virtual
// address space with no part being partially in the kernel address space
bool
is_valid_addr (const void *addr)
{
	bool is_valid = true;
	struct thread* t;
	void *page;

	// Validate that the given address is in the user virtual address space
	if (is_user_vaddr (addr))
		is_valid = false;

	// Validate that the given address is not also partially in kernel address space	
	if (is_kernel_vaddr (addr))
		is_valid = false;

	// Validate that the given address is indeed mapped to kernel space
	t = thread_current ();
	page = pagedir_get_page(t->pagedir, addr);
	if (page == NULL)
		is_valid = false;

	return is_valid;
}

// const struct intr_frame *f: 		the address of the interrupt frame
// int argc: 											the number of arguments within the interrupt frame
// char *argv[]: 								  an empty array that will be loaded with the arguments
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
		if (!is_valid_addr ((const void *) argv[i]))
			thread_exit ();
	}

	// Set a null pointer to mark the end of the command line arguments
	argv[argc] = NULL;
}

void 
halt (void)
{
	shutdown_power_off ();	
}

void 
exit (int status)
{
	struct thread *t = thread_current();
	printf("%s: exit(%d)\n", t->name, status);
	thread_exit ();	
}

pid_t
exec (const char *file)
{
	// TODO: add checks
	pid_t pid;
	pid = process_execute (file);
}

int
wait (pid_t pid)
{
	// TODO: implement process.c::process_wait
	return process_wait (pid);
}

bool
create (const char *file, unsigned initial_size)
{
	// Ensure calls to the filesystem are atomic
	bool is_created;
	lock_acquire (&filesys_lock);
	is_created = filesys_create (file, initial_size);
	lock_release (&filesys_lock);
	return is_created;
}

bool
remove (const char *file)
{
	// Ensure calls to the filesystem are atomic
	lock_acquire (&filesys_lock);
	return filesys_remove (file);	
	lock_release (&filesys_lock);
}

int
open (const char *file)
{
	// Open the given file. Ensure atomicity.
	lock_acquire (&filesys_lock);
	struct file *fd = filesys_open (file);
	lock_release (&filesys_lock);
}

int
filesize (int fd)
{
	return -1;
}

int
read (int fd, void *buffer, unsigned length)
{
	return -1;
}

int
write (int fd, const void *buffer, unsigned length)
{
	return -1;
}

void
seek (int fd, unsigned position)
{

}

unsigned
tell (int fd)
{
	return -1;
}

void
close (int fd)
{
	return -1;
}


