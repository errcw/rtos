/*
 * Serial server buffer (for input).
 */
#include <stdio.h>
#include <serialserver.h>
#include <syscall.h>

int main ()
{
  SerialMsg_t msg;
  SerialMsgQ_t msgq;

  /* set up the queue */
  msgq.size = 0;
  msgq.head = 0;
  msgq.tail = 0;

  /* the potentially waiting courier */
  int courier;

  for (;;) {
    int pid = Receive((char *)&msg, sizeof(msg));
    if (msg.cmd == SERIAL_GETBUF) {
      if (msgq.size > 0) {
        /* pull data out of the queue */
        msg.cmd = SERIAL_PUTSRV;
        msg.len = msgq.size;
        for (int i = 0; i < msgq.size; i++) {
          msg.msg[i] = msgq.msgs[ msgq.head ];
          msgq.head = (msgq.head + 1) % SERIAL_MSGQ_SIZE;
        }
        msgq.size = 0;
        /* reply to the courier */
        Reply(pid, (char *)&msg, msg.len+2);
        
      } else {
        /* block the courier until data is available */
        courier = pid;
      }
    } else if (msg.cmd == SERIAL_PUTBUF) {
      /* release the notifier */
      Reply(pid, 0, 0);

      if (courier) {
        /* release the data directly to the waiting courier */
        msg.cmd = SERIAL_PUTSRV;
        Reply(courier, (char *)&msg, msg.len+2);
        courier = 0; /* reset the courier */
      } else {
        /* insert the data into the queue */
        if (msgq.size < SERIAL_MSGQ_SIZE) {
          msgq.msgs[ msgq.tail ] = msg.msg[0];
          msgq.tail = (msgq.tail + 1) % SERIAL_MSGQ_SIZE;
          msgq.size = (msgq.size + 1);
        } else {
          printf("serial in buffer: queue overflow\n");
        }
      }
    } else {
      printf("serial in buffer: unknown command %d\n", msg.cmd);
    }
  }

  Exit();
}

