/*
 * The ender process.
 */
#include <syscall.h>
#include <terminal.h>

int main ()
{
  /* register ourselves as the ender */
  RegisterAs("ender");

  /* wait for our dead notifications */
  int dead[8];
  int num;
  for (int i = 0; i < 8; i++) {
    int eater = Receive((char *)&num, sizeof(num));
    dead[i] = num;
    Reply(eater, 0, 0);
  }

  /* print it all out */
  TerminalRCPrintf(12, 1, "Eaters died in the following order:");
  for (int i = 0; i < 8; i++) {
    TerminalRCPrintf(13, 1+3*i, "%d", dead[i]);
  }

  Exit();
}

