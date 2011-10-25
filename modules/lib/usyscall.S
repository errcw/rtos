/*
 * System calls (from the user side).
 */
  .file "usyscall.S"
  .text
  .align 4

#define STRIP_PROTOTYPES
#include <syscall.h>

/* Define a macro for system calls */
#define DEFSYSCALL(name, number)\
  .globl name ;\
  name: ;\
    pushl $number ;\
    int $INTERRUPT_SYSCALL ;\
    popl %eax ;\
    ret ;\

DEFSYSCALL(Create, SYSCALL_CREATE)
DEFSYSCALL(Destroy, SYSCALL_DESTROY)
DEFSYSCALL(Exit, SYSCALL_EXIT)
DEFSYSCALL(Pass, SYSCALL_PASS)

DEFSYSCALL(MyPid, SYSCALL_MYPID)
DEFSYSCALL(MyParentPid, SYSCALL_MYPARENTPID)
DEFSYSCALL(ValidPid, SYSCALL_VALIDPID)
DEFSYSCALL(MyPriority, SYSCALL_MYPRIORITY)

DEFSYSCALL(Send, SYSCALL_SEND)
DEFSYSCALL(Receive, SYSCALL_RECEIVE)
DEFSYSCALL(Reply, SYSCALL_REPLY)

DEFSYSCALL(RegisterAsNameserver, SYSCALL_REGISTERASNAMESERVER)
DEFSYSCALL(WhoIsNameserver, SYSCALL_WHOISNAMESERVER)

DEFSYSCALL(AwaitEvent, SYSCALL_AWAITEVENT)

DEFSYSCALL(DbgWrite, SYSCALL_DBGWRITE)

