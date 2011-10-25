/*
 * System call information.
 */
#ifndef __SYSCALL_H
#define __SYSCALL_H

/* System call numbers. */
#define SYSCALL_CREATE (0x01)
#define SYSCALL_DESTROY (0x02)
#define SYSCALL_EXIT (0x03)
#define SYSCALL_PASS (0x04)

#define SYSCALL_VALIDPID (0x05)
#define SYSCALL_MYPID (0x06)
#define SYSCALL_MYPARENTPID (0x07)
#define SYSCALL_MYPRIORITY (0x08)

#define SYSCALL_SEND (0x09)
#define SYSCALL_RECEIVE (0x0a)
#define SYSCALL_REPLY (0x0c)

#define SYSCALL_AWAITEVENT (0x0d)

#define SYSCALL_REGISTERAS (0x0e)
#define SYSCALL_REGISTERASNAMESERVER (0x0f)
#define SYSCALL_WHOIS (0x10)
#define SYSCALL_WHOISNAMESERVER (0x11)

#define SYSCALL_GET (0x12)
#define SYSCALL_PUT (0x13)
#define SYSCALL_DELAY (0x14)
#define SYSCALL_READ (0x15)
#define SYSCALL_WRITE (0x16)
#define SYSCALL_CONFIG (0x17)
#define SYSCALL_DBGWRITE (0x18)

/* The system call limits. */
#define SYSCALL_MIN (0x01)
#define SYSCALL_MAX (0x18)

/* The system call interrupt. */
#define INTERRUPT_SYSCALL (127)

/* System call prototypes. */
#ifndef STRIP_PROTOTYPES
typedef struct Time {
  int ms;
  int sec;
  int min;
} Time_t;

int Create (char *name, int priority);
int Destroy (int pid);
void Exit ();
void Pass ();

int ValidPid (int pid);
int MyPid ();
int MyParentPid ();
int MyPriority ();

int Send (int pid, char *message, int message_len, char *reply, int reply_len);
int Receive (char *message, int message_len);
int Reply (int pid, char *message, int message_len);

int AwaitEvent (int event);

int RegisterAs (char *name);
int RegisterAsNameserver ();
int WhoIs (char *name);
int WhoIsNameserver ();

char Get (int port);
int Put (int port, char byte);

int Delay (int tick);
Time_t GetTime ();

int Read (int port, char *buf, int nbytes);
int Write (int port, char *buf, int nbytes);

int Config (int port, int irq, int baud, int data, int parity, int stop);

int DbgWrite (char *buf, int nbytes);
#endif

#endif
