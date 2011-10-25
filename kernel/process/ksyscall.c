#include "ksyscall.h"
#include "interrupt.h"
#include "module.h"
#include "schedule.h"
#include "video.h"
#include "message.h"
#include "server/nameserver.h"
#include "server/timeserver.h"
#include "kern/error.h"

/* Offset to the system call number and return value. */
#define SYSCALL_NUM_OFFSET (13)
/* Offset to the system call arguments. */
#define SYSCALL_ARGS_OFFSET (15)

/* Sets the return for the active process. */
#define SYSCALL_RETURN(code)\
  SyscallSetReturn(activeproc, code);\
  return;

/* The pid of the nameserver. */
static Pid_t nameserver;

/* The processes waiting on each event. */
static Pid_t events[IRQ_NUM];

/*
 * Handles a system call. Examines the active process' stack to determine the
 * requested system call number then acts on the request.
 */
static void SyscallHandle (unsigned int intnum);

/* The system calls. */
static void SyscallCreate ();
static void SyscallDestroy ();
static void SyscallExit ();
static void SyscallPass ();

static void SyscallMyPid ();
static void SyscallMyParentPid ();
static void SyscallValidPid ();
static void SyscallMyPriority ();

static void SyscallSend ();
static void SyscallReceive ();
static void SyscallReply ();

static void SyscallRegisterAsNameserver ();
static void SyscallWhoIsNameserver ();

static void SyscallAwaitEvent ();
static void InterruptFired (unsigned int intnum);

static void SyscallDbgWrite ();

int SyscallInit ()
{
  return InterruptHook(INTERRUPT_SYSCALL, &SyscallHandle) &&
         MessageInit();
}

int SyscallGetType (Process_t *process)
{
  int *stack = (int *)PROC_TO_KERN(process, process->stackptr);
  return *(stack + SYSCALL_NUM_OFFSET);
}

int *SyscallGetArgs (Process_t *process)
{
  int *stack = (int *)PROC_TO_KERN(process, process->stackptr);
  return (stack + SYSCALL_ARGS_OFFSET);
}

void SyscallSetReturn (Process_t *process, int ret)
{
  int *stack = (int *)PROC_TO_KERN(process, process->stackptr);
  int *pret = (stack + SYSCALL_NUM_OFFSET);
  *pret = ret;
}

void SyscallHandle (unsigned int intnum)
{
  /* ensure we got a system call interrutpt */
  if (intnum != INTERRUPT_SYSCALL) {
    return;
  }

  /* check the validity of the system call number */
  int syscall = SyscallGetType(activeproc);
  if (syscall < SYSCALL_MIN || syscall > SYSCALL_MAX) {
    SYSCALL_RETURN(ENOSYS);
  }

  /* execute the system call */
  switch (syscall) {
    case SYSCALL_CREATE: SyscallCreate(); break;
    case SYSCALL_DESTROY: SyscallDestroy(); break;
    case SYSCALL_EXIT: SyscallExit(); break;
    case SYSCALL_PASS: SyscallPass(); break;

    case SYSCALL_MYPID: SyscallMyPid(); break;
    case SYSCALL_MYPARENTPID: SyscallMyParentPid(); break;
    case SYSCALL_VALIDPID: SyscallValidPid(); break;
    case SYSCALL_MYPRIORITY: SyscallMyPriority(); break;

    case SYSCALL_SEND: SyscallSend(); break;
    case SYSCALL_RECEIVE: SyscallReceive(); break;
    case SYSCALL_REPLY: SyscallReply(); break;

    case SYSCALL_REGISTERASNAMESERVER: SyscallRegisterAsNameserver(); break;
    case SYSCALL_WHOISNAMESERVER: SyscallWhoIsNameserver(); break;

    case SYSCALL_AWAITEVENT: SyscallAwaitEvent(); break;

    case SYSCALL_DBGWRITE: SyscallDbgWrite(); break;

    default:
      SyscallSetReturn(activeproc, ENOSYS);
  }
}

static void SyscallCreate ()
{
  int *args = SyscallGetArgs(activeproc);
  
  /* check the name */
  char *name = (char *)PROC_TO_KERN(activeproc, args[0]);
  if (!name) {
    SYSCALL_RETURN(EINVAL);
  }

  /* check the priority */
  int priority = args[1];
  if (priority < PRIORITY_MIN || priority > PRIORITY_MAX) {
    SYSCALL_RETURN(EINVAL);
  }

  /* find the module */
  ModuleInfo_t *mod_info = ModuleFind(name);
  if (!mod_info) {
    SYSCALL_RETURN(EINVAL);
  }

  /* create the process */
  Process_t *proc = ProcessCreate(mod_info);
  if (!proc) {
    SYSCALL_RETURN(ENOMEM);
  }
  proc->priority = priority;
  proc->parent_pid = activeproc->pid;

  /* schedule the process to run next */
  if (!ScheduleSetNext(proc)) {
    ProcessDestroy(proc); /* destroy */
    SYSCALL_RETURN(EFAIL);
  }

  /* return the new pid */
  SYSCALL_RETURN(proc->pid);
}

static void SyscallDestroy ()
{
  int pid = SyscallGetArgs(activeproc)[0];
  /* find the process to destroy */
  Process_t *proc = ProcessFind(pid);
  if (!proc) {
    SYSCALL_RETURN(EINVAL);
  }
  /* unschedule and destroy the process */
  MessageNotifyDestroyed(proc);
  ProcessDestroy(proc);
  SYSCALL_RETURN(EOK);
}

static void SyscallExit ()
{
  /* unschedule and destroy (ignoring error codes) */
  MessageNotifyDestroyed(activeproc);
  ProcessDestroy(activeproc);
  SYSCALL_RETURN(EFAIL); /* should never return */
}

static void SyscallPass ()
{
  /* no-op, we simply reschedule the active process */
  SYSCALL_RETURN(EOK);
}

static void SyscallMyPid ()
{
  SYSCALL_RETURN(activeproc->pid);
}

static void SyscallMyParentPid ()
{
  SYSCALL_RETURN(activeproc->parent_pid);
}

static void SyscallValidPid ()
{
  int *args = SyscallGetArgs(activeproc);
  Pid_t pid = (Pid_t)args[0];
  Process_t *proc = ProcessFind(pid);
  SYSCALL_RETURN(proc == 0 ? 0 : 1);
}

static void SyscallMyPriority ()
{
  SYSCALL_RETURN(activeproc->priority);
}

static void SyscallSend ()
{
  int *args = SyscallGetArgs(activeproc);

  Pid_t recv = (Pid_t)args[0];
  Process_t *recvproc = ProcessFind(recv);
  if (!recvproc) {
    SYSCALL_RETURN(EINVAL);
  }
  char *msg = (char *)PROC_TO_KERN(activeproc, args[1]);
  int msglen = args[2];
  if (msglen < 0) {
    SYSCALL_RETURN(EINVAL);
  }
  char *rpl = (char *)PROC_TO_KERN(activeproc, args[3]);
  int rpllen = args[4];
  if (rpllen < 0) {
    SYSCALL_RETURN(EINVAL);
  }

  SYSCALL_RETURN( MessageSend(recvproc, activeproc, msg, msglen, rpl, rpllen) );
}

static void SyscallReceive ()
{
  int *args = SyscallGetArgs(activeproc);

  char *msg = (char *)PROC_TO_KERN(activeproc, args[0]);
  int msglen = args[1];
  if (msglen < 0) {
    SYSCALL_RETURN(EINVAL);
  }

  SYSCALL_RETURN( MessageReceive(activeproc, msg, msglen) );
}

static void SyscallReply ()
{
  int *args = SyscallGetArgs(activeproc);

  Pid_t send = (Pid_t)args[0];
  Process_t *sendproc = ProcessFind(send);
  if (!sendproc) {
    SYSCALL_RETURN(EINVAL);
  }
  char *rpl = (char *)PROC_TO_KERN(activeproc, args[1]);
  int rpllen = args[2];
  if (rpllen < 0) {
    SYSCALL_RETURN(EINVAL);
  }

  SYSCALL_RETURN( MessageReply(sendproc, activeproc, rpl, rpllen) );
}

static void SyscallRegisterAsNameserver ()
{
  nameserver = activeproc->pid;
  SYSCALL_RETURN(EOK);
}

static void SyscallWhoIsNameserver ()
{
  SYSCALL_RETURN(nameserver);
}

static void SyscallAwaitEvent ()
{
  int *args = SyscallGetArgs(activeproc);

  /* check the interrupt number */
  int irq = args[0];
  if (irq < 0 || irq >= IRQ_NUM) {
    SYSCALL_RETURN(EINVAL);
  }

  /* check there is not already someone waiting */
  if (events[irq]) {
    SYSCALL_RETURN(ENOEVT);
  }

  /* wait for the interrupt */
  InterruptHook(IRQ_BASE + irq, &InterruptFired);
  events[irq] = activeproc->pid;
  activeproc->state = BLOCKED_INTERRUPT;

  SYSCALL_RETURN(EOK);
}

static void InterruptFired (unsigned int intnum)
{
  /* find the waiting process */
  Pid_t pid = events[intnum - IRQ_BASE];
  if (!pid) {
    return;
  }
  Process_t *proc = ProcessFind(pid);
  if (!proc) {
    return;
  }
  /* force the process to execute next */
  ScheduleSetNext(proc);
  /* clear the interrupt */
  events[intnum - IRQ_BASE] = 0;
  InterruptUnhook(intnum);
}

static void SyscallDbgWrite ()
{
  int *args = SyscallGetArgs(activeproc);

  /* check the pointer */
  char *buf = (char *)PROC_TO_KERN(activeproc, args[0]);
  if (!buf) {
    SYSCALL_RETURN(EINVAL);
  }

  /* check the number of bytes */
  int nbytes = args[1];
  if (nbytes < 0) {
    SYSCALL_RETURN(EINVAL);
  }

  /* write the characters */
  for (int i = 0; i < nbytes; i++) {
    VideoPut(buf[i]);
  }

  SYSCALL_RETURN(nbytes);
}

