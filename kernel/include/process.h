/*
 * Manages processes.
 */
#ifndef __PROCESS_H
#define __PROCESS_H

#include "module.h"
#include "memory.h"
#include "kern/limits.h"

/* Each process is given 4M of memory (stack + global). */
#define PROCESS_MEMORY (0x400000)

/* A process id type. */
typedef unsigned int Pid_t;

/* Return the index from the pid. */
#define PID_INDEX(pid) ((pid) & 0xff)

/* Return the generation from the pid. */
#define PID_GENERATION(pid) (((pid) >> 8) & 0xffffff)

/* Creates a pid from an index and generation. */
#define PID_CREATE(index,generation) ((((generation)&0xffffff)<<8)|((index)&0xff))

/* Convert an address from process to kernel space. */
#define PROC_TO_KERN(proc, addr) \
  (addr + (MEMORY_TOP - (PID_INDEX((proc)->pid)+1) * PROCESS_MEMORY))

/* Convert an address from kernel to process space. */
#define KERN_TO_PROC(proc, addr) \
  (addr - (MEMORY_TOP - (PID_INDEX((proc)->pid)+1) * PROCESS_MEMORY))

/* A process state. */
typedef enum ProcessState {
  ACTIVE,
  READY,
  BLOCKED_SEND,
  BLOCKED_RECEIVE,
  BLOCKED_REPLY,
  BLOCKED_INTERRUPT,
  DEAD
} ProcessState_t;

/* A process. */
typedef struct Process {
  unsigned int stackptr; /* the stack pointer (relative to the data segment) */

  SegmentId_t dataseg; /* the data segment */
  SegmentId_t codeseg; /* the code segment */

  ProcessState_t state; /* the current state */
  unsigned int priority; /* the priority */

  Pid_t pid; /* the process id */
  Pid_t parent_pid; /* the parent process id */
} Process_t;

/* The currently active process. */
extern Process_t *activeproc;

/*
 * Initialises process handling.
 */
int ProcessInit ();

/*
 * Creates a process to execute the given module.
 */
Process_t *ProcessCreate (ModuleInfo_t *mod);

/*
 * Finds a process based on its pid, or returns null if no such process exists.
 */
Process_t *ProcessFind (Pid_t pid);

/*
 * Kills a process.
 */
int ProcessDestroy (Process_t *proc);


#endif

