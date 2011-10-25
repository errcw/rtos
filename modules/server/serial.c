/*
 * The serial server.
 */
#include <stdio.h>
#include <string.h>

#include <serialserver.h>
#include <syscall.h>
#include <irq.h>

/* Input and output queues. */
static SerialMsgQ_t inq;
static SerialMsgQ_t outq;

/* Processes waiting for data. */
#define GETTERS_MAX (32)
struct {
  unsigned int head;
  unsigned int tail;
  unsigned int size;
  int pids[GETTERS_MAX];
} getters;

/* The in- and out-bound couriers. */
static int courier_in;
static int courier_out;

/* Set up and run the server. */
static void Init ();
static void Run ();

/* Handle IO requests. */
static void HandleGet (int getter_pid);
static void HandlePut (int putter_pid, char *bytes, int nbytes);

/* Provide as much data as possible to any processes waiting on Get. */
static void ServiceGetters ();
/* Provide as much data as possible to the outbound courier. */
static void ServicePutCourier ();
/* Pull data from the inbound courier to the input queue. */
static void ServiceGetCourier (char *bytes, int nbytes);

static int min (int x, int y);

int main ()
{
  Init();
  Run();
  Exit();
}

static void Init ()
{
  /* initialize the queues */
  inq.head = 0;
  inq.tail = 0;
  outq.size = 0;
  outq.head = 0;
  outq.tail = 0;
  getters.head = 0;
  getters.tail = 0;
  getters.size = 0;

  SerialCfg_t cfg;
  int creator = Receive((char *)&cfg, sizeof(cfg));

  /* register ourselves */
  char name[128];
  sprintf(name, SERIALSERVER_NAME_PREFIX"%d", cfg.port);
  RegisterAs(name);

  /* create the notifier, buffers, and courier */
  int server_pid = MyPid();
  int high_pri = MyPriority() + 1;
  int notifier_pid = Create("serial_notifier", high_pri);
  int buffer_in_pid = Create("serial_buffer_in", high_pri);
  int buffer_out_pid = Create("serial_buffer_out", high_pri);
  courier_in = Create("serial_courier", high_pri);
  courier_out = Create("serial_courier", high_pri);

  /* initialize them */
  SerialNotifierCfg_t ncfg = { buffer_in_pid, buffer_out_pid, cfg };
  Send(notifier_pid, (char *)&ncfg, sizeof(ncfg), 0, 0);

  SerialCourierCfg_t in_cfg = { buffer_in_pid, server_pid, SERIAL_GETBUF };
  Send(courier_in, (char *)&in_cfg, sizeof(in_cfg), 0, 0);

  SerialCourierCfg_t out_cfg = { server_pid, buffer_out_pid, SERIAL_GETSRV };
  Send(courier_out, (char *)&out_cfg, sizeof(out_cfg), 0, 0);

  /* let our creator know we're alive */
  Reply(creator, 0, 0);
}

static void Run ()
{
  SerialMsg_t msg;

  for (;;) {
    int pid = Receive((char *)&msg, sizeof(msg));
    switch (msg.cmd) {
      /* handle the user commands */
      case SERIAL_GET:
      case SERIAL_READ: HandleGet(pid); break;
      case SERIAL_PUT:
      case SERIAL_WRITE: HandlePut(pid, msg.msg, msg.len); break;

      /* handle the internal commands from couriers */
      case SERIAL_GETSRV: ServicePutCourier(); break;
      case SERIAL_PUTSRV: ServiceGetCourier(msg.msg, msg.len); break;

      default:
        printf("serial server: unknown command %d\n", msg.cmd);
    }
  }
}

static void HandleGet (int getter_pid)
{
  if (inq.size > 0) {
    /* pass back the data */
    char data = inq.msgs[ inq.head ];
    inq.head = (inq.head + 1) % SERIAL_MSGQ_SIZE;
    inq.size = (inq.size - 1);
    Reply(getter_pid, &data, sizeof(data));
    
  } else {
    if (getters.size < GETTERS_MAX) {
      /* queue the getter */
      getters.pids[getters.tail] = getter_pid;
      getters.tail = (getters.tail + 1) % GETTERS_MAX;
      getters.size = (getters.size + 1);
    } else {
      /* fail the getter */
      Reply(getter_pid, 0, 0);
    }

  }
}

static void HandlePut (int putter_pid, char *bytes, int nbytes)
{
  /* inform the putter how much data will be written */
  int outbytes = min(nbytes, SERIAL_MSGQ_SIZE - outq.size);
  Reply(putter_pid, (char *)&outbytes, sizeof(outbytes));

  /* enqueue the data */
  while (outbytes) {
    outq.msgs[ outq.tail ] = *bytes;
    outq.tail = (outq.tail + 1) % SERIAL_MSGQ_SIZE;
    outq.size = (outq.size + 1);
    bytes++;
    outbytes--;
  }

  /* try and service the put courier */
  ServicePutCourier();
}

static void ServiceGetters ()
{
  while (getters.size > 0 && inq.size > 0) {
    /* grab a getter from the queue */
    int pid = getters.pids[getters.head];
    getters.head = (getters.head + 1) % GETTERS_MAX;
    getters.size = (getters.size - 1);

    /* grab data from the input queue */
    char data = inq.msgs[ inq.head ];
    inq.head = (inq.head + 1) % SERIAL_MSGQ_SIZE;
    inq.size = (inq.size - 1);

    /* pass the data back to the getter */
    Reply(pid, &data, sizeof(data));
  }
}

static void ServicePutCourier ()
{
  /* check how many bytes we can send to the outbound courier */
  int bytes = min(outq.size, SERIAL_MSGBUF_SIZE);
  if (bytes) {
    SerialMsg_t msg;
    msg.cmd = SERIAL_PUTBUF;
    msg.len = bytes;
    /* pull data out of the out queue */
    for (int i = 0; i < bytes; i++) {
      msg.msg[i] = outq.msgs[ outq.head ];
      outq.head = (outq.head + 1) % SERIAL_MSGQ_SIZE;
    }
    outq.size -= bytes;
    /* pass data off to the outbound courier */
    Reply(courier_out, (char *)&msg, msg.len+2);
  }
}

static void ServiceGetCourier (char *bytes, int nbytes)
{
  /* release the courier */
  Reply(courier_in, 0, 0);
  /* write into the input queue */
  for (int i = 0; i < nbytes; i++) {
    inq.msgs[ inq.tail ] = bytes[i];
    inq.tail = (inq.tail + 1) % SERIAL_MSGQ_SIZE;
  }
  inq.size = min(inq.size + nbytes, SERIAL_MSGQ_SIZE);
  /* check if we can pass data back */
  ServiceGetters();
}

static int min (int x, int y)
{
  return (x < y) ? x : y;
}

