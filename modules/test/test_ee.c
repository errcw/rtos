/*
 * The eater/ender test parent.
 */
#include <syscall.h>
#include <terminal.h>

int main ()
{
  /* print the grid */
  for (int i = 1; i <= 100; i++) {
    TerminalRCPrintf(1+(i-1)/10, 1+3*((i-1)%10), "%3d", i);
  }

  int pri = MyPriority();

  /* create the eaters */
  for (int i = 2; i <= 9; i++) {
    int eater = Create("test_eater", pri);
    Send(eater, (char *)&i, sizeof(i), 0, 0);
  }
  /* create the ender */
  Create("test_ender", pri);

  Exit();
}

