#include "process.h"

#define EFLAGS_INIT (0x00000202)

/* Our processes. */
static Process_t processes[PROCESS_MAX];

Process_t *activeproc;

/* The process free list. */
static unsigned int process_free[PROCESS_MAX];
static int process_free_top;

/* Initializes a process in memory. */
static int ProcessStart (Process_t *proc, ModuleInfo_t *mod);


int ProcessInit ()
{
  for (int i = 0; i < PROCESS_MAX; i++) {
    processes[i].pid = 0;
    processes[i].state = DEAD;
    process_free[i] = i;
  }
  process_free_top = 0;
  return 1;
}

Process_t *ProcessCreate (ModuleInfo_t *mod)
{
  /* make sure we have a valid module pointer */
  if (!mod || !mod->code_start) {
    return 0;
  }
  /* make sure we have processes to spare */
  if (process_free_top >= PROCESS_MAX) {
    return 0;
  }
  /* allocate a descriptor and initialize it */
  unsigned int pidx = process_free[process_free_top++];
  Process_t *proc = &processes[pidx];
  proc->pid = PID_CREATE(pidx, PID_GENERATION(proc->pid)+1);
  if (ProcessStart(proc, mod)) {
    return proc;
  } else {
    /* start failed, reclaim the descriptor */
    process_free[--process_free_top] = pidx;
    return 0;
  }
}

Process_t *ProcessFind (Pid_t pid)
{
  if (pid == 0) {
    return 0;
  }
  unsigned int idx = PID_INDEX(pid);
  if (idx > PROCESS_MAX) {
    return 0;
  }
  Process_t *proc = &processes[idx];
  if (proc->state == DEAD) {
    return 0;
  }
  return proc;
}

int ProcessDestroy (Process_t *proc)
{
  /* ensure we can kill the given process */
  if (!proc || !proc->pid || proc->state == DEAD) {
    return 0;
  }
  /* kill it */
  proc->state = DEAD;
  /* reclaim the descriptor */
  process_free[--process_free_top] = PID_INDEX(proc->pid);
  return 1;
}

static int ProcessStart (Process_t *proc, ModuleInfo_t *mod)
{
  /* process memory */
  unsigned int membase = PROC_TO_KERN(proc, 0x0);

  /* create the segments */
  Segment_t seg;
  seg.start = mod->code_start;
  seg.limit = mod->code_size;
  seg.type = SEGMENT_E;
  seg.ring = SEGMENT_RING0;
  proc->codeseg = MemoryCreateSegment(&seg);
  if (!proc->codeseg) {
    return 0;
  }
  seg.start = membase;
  seg.limit = PROCESS_MEMORY - 1;
  seg.type = SEGMENT_RW;
  seg.ring = SEGMENT_RING0;
  proc->dataseg = MemoryCreateSegment(&seg);
  if (!proc->dataseg) {
    MemoryFreeSegment(proc->codeseg);
    return 0;
  }

  /* initialize the stack */
  int *stack = (int *)(membase + PROCESS_MEMORY) - 1;

  /* set up for data copy inside user code */
  *(stack--) = mod->data_size - mod->data_init_size;
  *(stack--) = mod->data_init_size;
  *(stack--) = mod->data_start;
  *(stack--) = SEGMENT_KERNEL_DATA;

  /* set up for return from interrupt */
  *(stack--) = EFLAGS_INIT; /* eflags */
  *(stack--) = proc->codeseg; /* cs */
  *(stack--) = mod->code_entry; /* eip */
  *(stack--) = 0; /* int number */
  *(stack--) = 0; /* eax */
  *(stack--) = 0; /* ebx */
  *(stack--) = 0; /* ecx */
  *(stack--) = 0; /* edx */
  *(stack--) = 0; /* esi */
  *(stack--) = 0; /* edi */
  *(stack--) = 0; /* ebp */
  *(stack--) = ((proc->dataseg & 0xffff) << 16) | (proc->dataseg & 0xffff); /* ds, es */
  *(stack) = ((proc->dataseg & 0xffff) << 16) | (proc->dataseg & 0xffff); /* fs, gs */

  /* set the stack pointer (remembering to make it relative to the segment) */
  proc->stackptr = KERN_TO_PROC(proc, (unsigned int)stack);
  for (int i=1; i<28; i++) {
    *(stack-i) = 0;
  }
  *(stack-27) = 895;
  *(stack-25) = 65535;

  return 1;
}

