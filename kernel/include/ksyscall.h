/*
 * System calls.
 */
#ifndef __KSYSCALL_H
#define __KSYSCALL_H

#include "process.h"
#include "kern/syscall.h"

/*
 * Initializes system call handling.
 */
int SyscallInit ();

/*
 * Returns the type of system call requested by the given process.
 */
int SyscallGetType (Process_t *process);

/*
 * Returns a pointer to the arguments of the last system call for the given process.
 */
int *SyscallGetArgs (Process_t *process);

/*
 * Sets the system call return value for a given process.
 */
void SyscallSetReturn (Process_t *process, int ret);

#endif

