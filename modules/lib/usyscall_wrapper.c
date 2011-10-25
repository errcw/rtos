/*
 * Implementations of high-level system calls.
 */
#include <stdio.h>

#include <syscall.h>
#include <limits.h>
#include <error.h>

#include <nameserver.h>
#include <timeserver.h>
#include <serialserver.h>

/* Server process ids. */
static int nameserver = 0;
static int timeserver = 0;

/* Sends a message to the name server. */
static int NSSend (char msgtype, char *name);

/* Safe string copy up until a specified buffer size. */
static int safe_strcpy (char *dest, char *src, int maxlen, int *gotlen);

/* Finds the serial server for the given port. */
static int GetSerialServer (int port);

int RegisterAs (char *name)
{
  return NSSend(NAMESERVER_REGISTER, name);
}

int WhoIs (char *name)
{
  return NSSend(NAMESERVER_WHOIS, name);
}

int Delay (int ticks)
{
  if (ticks < 0) {
    /* negative delays are invalid */
    return EINVAL;
  } else if (ticks == 0) {
    /* a zero delay returns immediately */
    return EOK;
  }

  if (!timeserver) {
    timeserver = WhoIs(TIMESERVER_NAME);
  }

  /* call the delay */
  return Send(timeserver, (char *)&ticks, sizeof(ticks), 0, 0);
}

Time_t GetTime ()
{
  if (!timeserver) {
    timeserver = WhoIs(TIMESERVER_NAME);
  }

  const int op = TIMESERVER_GETTIME;
  Time_t time;

  Send(timeserver, (char *)&op, sizeof(op), (char *)&time, sizeof(time));
  return time;
}

static int NSSend (char msgtype, char *name)
{
  /* find the nameserver */
  if (!nameserver) {
    nameserver = WhoIsNameserver();
  }

  char msg[NAME_MAXLEN + 1];
  int msglen;

  /* create the message */
  msg[0] = msgtype;
  if (safe_strcpy(&msg[1], name, NAME_MAXLEN+1, &msglen) < 0) {
    return EN2BIG;
  }

  /* send to the name server */
  int reply;
  int result = Send(nameserver, msg, msglen+1, (char *)&reply, sizeof(reply));
  if (result < 0) {
    return result;
  }
  return reply;
}


char Get (int port)
{
  int pid = GetSerialServer(port);
  if (pid <= 0) {
    return pid;
  }

  char cmd = SERIAL_GET;
  char ret;

  int result = Send(pid, (char *)&cmd, sizeof(cmd), &ret, sizeof(ret));
  if (result < 0) {
    return result;
  }

  return ret;
}

int Put (int port, char byte)
{
  int pid = GetSerialServer(port);
  if (pid <= 0) {
    return pid;
  }

  SerialMsg_t msg;
  msg.cmd = SERIAL_PUT;
  msg.len = 1;
  msg.msg[0] = byte;

  int putbytes;

  int result = Send(pid, (char *)&msg, sizeof(msg), (char *)&putbytes, sizeof(putbytes));
  if (result < 0) {
    return result;
  }
  return putbytes;
}

int Write (int port, char *buf, int nbytes)
{
  int pid = GetSerialServer(port);
  if (pid <= 0) {
    return pid;
  }

  int putbytes = (nbytes > SERIAL_MSGQ_SIZE) ? SERIAL_MSGQ_SIZE : nbytes;

  SerialMsg_t msg;
  msg.cmd = SERIAL_PUT;
  msg.len = putbytes;
  for (int i = 0; i < putbytes; i++) {
    msg.msg[i] = buf[i];
  }

  int result = Send(pid, (char *)&msg, sizeof(msg), (char *)&putbytes, sizeof(putbytes));
  if (result < 0) {
    return result;
  }
  return putbytes;
}

int Config (int port, int irq, int baud, int data, int parity, int stop)
{
  /* quickly check for clearly invalid values */
  if (port < 0 || irq < 0 || baud < 0 || data < 0 || parity < 0 || stop < 0) {
    return EINVAL;
  }

  /* check the serial server does not already exist */
  if (ValidPid(GetSerialServer(port))) {
    return EINVAL;
  }

  /* create and configure a server */
  int serial_pid = Create("serial", SERIALSERVER_PRIORITY);
  if (serial_pid <= 0) {
    return serial_pid;
  }

  //SerialCfg_t cfg = { port, irq, baud, data, parity, stop }; //TODO causes data corruption
  SerialCfg_t cfg;
  cfg.port = port;
  cfg.irq = irq;
  cfg.baud = baud;
  cfg.data = data;
  cfg.parity = parity;
  cfg.stop = stop;
  return Send(serial_pid, (char *)&cfg, sizeof(cfg), 0, 0);
}

static int GetSerialServer (int port)
{
  char name[128];
  sprintf(name, SERIALSERVER_NAME_PREFIX"%d", port);

  int pid = WhoIs(name);
  return pid;
}

static int safe_strcpy (char *dest, char *src, int maxlen, int *gotlen)
{
  for (int i = 0; i < maxlen; i++) {
    dest[i] = src[i];
    if (src[i] == 0) {
      if (gotlen != 0) {
        *gotlen = i + 1;
      }
      return EOK;
    }
  }
  return EFAIL;
}

