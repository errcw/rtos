/*
 * Serial server notifier.
 */
#include <stdio.h>
#include <string.h>

#include <serialserver.h>
#include <syscall.h>
#include <irq.h>

#include <cs452/machine/cpufunc.h>
#include <cs452/machine/pio.h>
#include <cs452/machine/serial.h>

#define INT_LSR (3)
#define INT_RECVAVAIL (2)
#define INT_THR (1)
#define INT_MSR (0)

/* Our configuration. */
static SerialNotifierCfg_t cfg;
static int port;

/* If the serial port is ready for data (clear to send, transmit register empty). */
static int serial_cts;
static int serial_rdy;

/* Set up and run the buffer */
static void Init ();
static void Run ();

static void SerialInit ();
static void SerialHandleInt (int int_type);
static void SerialSend ();
static void SerialReceive ();

int main()
{
  Init();
  Run();
  Exit();
}

static void Init ()
{
  int server_pid = Receive((char *)&cfg, sizeof(cfg));

  port = cfg.port_cfg.port;
  serial_cts = 1;
  serial_rdy = 0;
  SerialInit();

  /* disable interrupts inside this task */
  disable_intr();
  
  Reply(server_pid, 0, 0);
}

static void Run () 
{
  for (;;){
    AwaitEvent(cfg.port_cfg.irq);
    for (;;) {
      char iir = inb(port + USART_IIR) & 0x7;
      if (iir & 1) {
        break;
      }
      char int_type = iir >> 1;
      SerialHandleInt(int_type);
    }
  }
}

static void SerialInit ()
{
  unsigned int mode_word;

  unsigned int stop = ((cfg.port_cfg.stop - 1) & 1);
  stop <<= 2;

  unsigned int baud = cfg.port_cfg.baud / 10;
  baud = 11520 / baud;

  unsigned int parity = cfg.port_cfg.parity << 3;
  parity &= 0x18;

  unsigned int data = cfg.port_cfg.data - 5;
  data &= 3;

  mode_word = data | stop | parity;

  outb(port + USART_LCR, inb(port + USART_LCR) | LCR_DLAB);
  outb(port, baud % 256);
  outb(port + 1, baud / 256);
  outb(port + USART_LCR, mode_word & 0x7F);

  /* turn on DTR and all other outputs (harmless) */
  outb(port + USART_MCR, 0xF);

  /* turn on all interrupt sources */
  outb(port + USART_IER, 0xF);

  /* read pending input and status bits (ignore them) */
  inb(port + USART_LSR);
  inb(port + USART_MSR);
  inb(port);
}

static void SerialHandleInt (int int_type)
{
  int reg_data = 0;

  switch (int_type) {
    case INT_LSR:
      reg_data = inb(port + USART_LSR);
      printf("serial notifier: LSR (%d)\n", reg_data);
      break;

    case INT_RECVAVAIL:
      SerialReceive();
      break;

    case INT_THR:
      serial_rdy = 1;
      SerialSend();
      break;

    case INT_MSR:
      reg_data = inb(port + USART_MSR) & 0xFF;
      if (reg_data & MSR_Delta_CTS) {
        if (reg_data & MSR_CTS) {
          /* serial port is ready for data */
          serial_cts = 1;
          SerialSend();
        } else {
          /* wait until we are able to send again */
          serial_cts = 0;
        }
      }
      break;

    default:
      printf("serial notifier: unrecognized interrupt %d\n", int_type);
      break;
  }
}

static void SerialReceive ()
{
  /* read the data */
  char data = inb(port + USART_RBR);

  /* punt it to the buffer */
  SerialMsg_t msg;
  msg.cmd = SERIAL_PUTBUF;
  msg.len = 1;
  msg.msg[0] = data;
  Send(cfg.buffer_in_pid, (char *)&msg, msg.len+2, 0, 0);
}

static void SerialSend ()
{
  if (serial_cts && serial_rdy) {
    /* ask the buffer to send the data */
    SerialMsg_t msg;
    msg.cmd = SERIAL_GETBUF;
    msg.len = sizeof(int);
    *((int *)msg.msg) = (int)(port + USART_THR);
    Send(cfg.buffer_out_pid, (char *)&msg, msg.len+2, 0, 0);
    /* wait for another THR interrupt */
    serial_rdy = 0;
  }
}

