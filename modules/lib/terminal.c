#include <stdio.h>
#include <stdarg.h>

#include <terminal.h>
#include <syscall.h>
#include <irq.h>
#include <cs452/machine/serial.h>

/* Buffer for printf. */
#define PRINTF_BUFSIZE (512)
static char printf_buffer[PRINTF_BUFSIZE];
static char printf_buffer_rc[PRINTF_BUFSIZE];

#define ESC "\033"

#define TERMINAL (USART_1_BASE)

void TerminalInit ()
{
  Config(TERMINAL, IRQ_SERIAL1, 9600, 8, 0, 1);
}

void TerminalClear ()
{
  TerminalPrintf(ESC"c");
  TerminalPrintf(ESC"[7l");
  TerminalPrintf(ESC"[2j");
  TerminalPrintf(ESC"[?25h");
}

int TerminalPrintf (const char *format, ...)
{
  va_list args;
  va_start(args, format);
  int bytes = vsprintf(printf_buffer, format, args);
  va_end(args);

  return Write(TERMINAL, printf_buffer, bytes);
}

int TerminalRCPrintf (unsigned int r, unsigned int c, const char *format, ...)
{
  va_list args;
  va_start(args, format);
  vsprintf(printf_buffer_rc, format, args);
  va_end(args);

  return TerminalPrintf(ESC"[%d;%dH%s", r, c, printf_buffer_rc);
}

char TerminalRead ()
{
  return Get(TERMINAL);
}

char *TerminalReadLine (int echox, int echoy)
{
  static char cmd_buffer[TERMINAL_LINE_MAX+1];
  unsigned int bufsize = 0;
  for(;;) {
    char in = TerminalRead();
    if (in == '\177') {
      if (bufsize > 0) {
        TerminalRCPrintf(echox, echoy+bufsize, " ");
        bufsize--;
      }
    } else if (in == '\015') {
      cmd_buffer[bufsize++] = '\0';
      break;
    } else {
      if (bufsize < TERMINAL_LINE_MAX) {
        cmd_buffer[bufsize++] = in;
        TerminalRCPrintf(echox, echoy+bufsize, "%c", in); /* echo the command */
      }
    }
  } 
  TerminalRCPrintf(echox, echoy, "%40s", " ");
  return cmd_buffer;
}

void TerminalSetColours (int fg, int bg)
{
  TerminalPrintf(ESC"[%d;%dm", fg, bg);
}

