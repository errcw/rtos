/*
 * Writes the time in the top right corner.
 */
#include <stdio.h>

#include <syscall.h>
#include <terminal.h>

#define TICKS_PER_100MS (100)

int main ()
{
  RegisterAs("time_writer");

  Time_t time;
  for (;;) {
    time = GetTime();
    TerminalRCPrintf(0, 60, "Time: %2d:%02d.%d", time.min, time.sec, time.ms/100);
    Delay(TICKS_PER_100MS);
  }
  Exit();
}

