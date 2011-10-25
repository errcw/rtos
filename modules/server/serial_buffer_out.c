/*
 * Serial server buffer (for output).
 */
#include <stdio.h>
#include <serialserver.h>
#include <syscall.h>

#include <cs452/machine/cpufunc.h>
#include <cs452/machine/pio.h>

int main ()
{
  SerialMsg_t msg;
  SerialMsgQ_t msgq;

  /* set up the queue */
  msgq.size = 0;
  msgq.head = 0;
  msgq.tail = 0;

  /* if can need to transmit right away */
  int transmit_now = 0;
  /* the port to transmit */
  int port;

  for (;;) {
    int pid = Receive((char *)&msg, sizeof(msg));
    Reply(pid, 0, 0); /* release immediately */

    if (msg.cmd == SERIAL_GETBUF) {
      port = *((int *)msg.msg);
      if (msgq.size > 0) {
        /* data ready to transmit */
        char data = msgq.msgs[ msgq.head ];
        msgq.head = (msgq.head + 1) % SERIAL_MSGQ_SIZE;
        msgq.size = (msgq.size - 1);
        /* write the byte */
        outb(port, data);
      } else {
        /* note we are able to transmit */
        transmit_now = 1;
      }
    } else if (msg.cmd == SERIAL_PUTBUF) {
      char *bytes = &(msg.msg[0]);
      if (transmit_now) {
        /* write the byte */
        outb(port, *bytes++);
        msg.len -= 1;
        /* cannot write any more (right now) */
        transmit_now = 0;
      }
      /* enqueue the rest */
      for (int i = 0; i < msg.len && msgq.size < SERIAL_MSGQ_SIZE; i++) {
        msgq.msgs[ msgq.tail ] = bytes[i];
        msgq.tail = (msgq.tail + 1) % SERIAL_MSGQ_SIZE;
        msgq.size = (msgq.size + 1);
      }
    } else {
      printf("serial out buffer: unknown command %d\n", msg.cmd);
    }
  }
}

