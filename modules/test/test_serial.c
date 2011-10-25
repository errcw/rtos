/*
 * Test serial read/write.
 */
#include <stdio.h>

#include <syscall.h>
#include <irq.h>
#include <terminal.h>

#include <cs452/cs452.h>
#include <cs452/machine/serial.h>

int main ()
{
  char in;
  printf("TEST: serial server\n");

  Config(USART_1_BASE, IRQ_SERIAL1, 9600, 8, 0, 1);
  Put(USART_1_BASE, 'H');
  Put(USART_1_BASE, 'e');
  Put(USART_1_BASE, 'l');
  Put(USART_1_BASE, 'l');
  Put(USART_1_BASE, 'o');
  for (;;) {
    in = Get(USART_1_BASE);
    if(in == 'q')
      break;
    Put(USART_1_BASE, in);
  }
  
  TerminalClear();
  char name[] = "DANIEL";
  Write(USART_1_BASE, name, sizeof(name) );
  for (;;) {
    in = TerminalRead();
    if(in == 'q')
      break;
    TerminalPrintf("%c", in);
  }
  TerminalRCPrintf(10,10, "THIS IS THE END\n");

  printf("DONE\n");
  Exit();
}

