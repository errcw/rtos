/*
 * Time test.
 */
#include <stdio.h>
#include <syscall.h>

int main ()
{
  printf("TEST: time server\n");

  printf("delaying for 1 tick...");
  if (Delay(1) < 0) {
    printf("failed\n");
    Exit();
  }
  printf("ok\n");

  printf("delaying for 20 ticks (1 second)...");
  if (Delay(20) < 0) {
    printf("failed\n");
    Exit();
  }
  printf("ok\n");

  printf("delaying for 300 ticks (30 seconds)...");
  if (Delay(300) < 0) {
    printf("failed\n");
    Exit();
  }
  printf("ok\n");

  printf("DONE\n");

  Exit();
}

