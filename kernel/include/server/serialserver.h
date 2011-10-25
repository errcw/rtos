/*
 * Interface for communicating with the serial server.
 */
#ifndef __SERIALSERVER_H
#define __SERIALSERVER_H

/* The name used by the serial server to register itself. */
#define SERIALSERVER_NAME_PREFIX "ss-"

/* The priority of a serial server. */
#define SERIALSERVER_PRIORITY (17)

/* Commands. */
#define SERIAL_GET (5) /* user-facing */
#define SERIAL_PUT (6)
#define SERIAL_READ (7)
#define SERIAL_WRITE (8)

#define SERIAL_GETBUF (1) /* internal */
#define SERIAL_PUTBUF (2)
#define SERIAL_GETSRV (3)
#define SERIAL_PUTSRV (4)

/* Initial serial server configuration message. */
typedef struct SerialCfg {
  unsigned int irq;
  unsigned int port;
  unsigned int baud;
  unsigned int data;
  unsigned int parity;
  unsigned int stop;
} SerialCfg_t;

/* Serial notifier configuration message. */
typedef struct SerialNotifierCfg {
  /* the buffers */
  int buffer_in_pid;
  int buffer_out_pid;
  /* the port config */
  SerialCfg_t port_cfg;
} SerialNotifierCfg_t;

/* Serial courier configuration message. */
typedef struct SerialCourierCfg {
  int src_pid;
  int dst_pid;
  char cmd;
} SerialCourierCfg_t;

/* Serial data message. */
#define SERIAL_MSGBUF_SIZE (1024)
typedef struct SerialMsg {
  char cmd;
  char len;
  char msg[SERIAL_MSGBUF_SIZE];
} SerialMsg_t;

/* Serial data message queue. */
#define SERIAL_MSGQ_SIZE (4096)
typedef struct SerialMsgQ {
  unsigned int head;
  unsigned int tail;
  unsigned int size;
  char msgs[SERIAL_MSGQ_SIZE];
} SerialMsgQ_t;

#endif

