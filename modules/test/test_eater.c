/*
 * The eater process.
 */
#include <syscall.h>
#include <terminal.h>
#include <timeserver.h>

#define DELAY_SEC (1000)

int main ()
{
  int num;

  /* find out our number */
  int creator = Receive((char *)&num, sizeof(num));
  Reply(creator, 0, 0);

  /* do the eating */
  for (int i = num; i <= 100; i += num) {
    Delay(DELAY_SEC * num);
    TerminalRCPrintf(1+(i-1)/10, 1+3*((i-1)%10), "    ");
  }

  /* notify the ender we finished */
  Send(WhoIs("ender"), (char *)&num, sizeof(num), 0, 0);

  Exit();
}

