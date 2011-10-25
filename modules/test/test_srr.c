/*
 * Bootstrap test process.
 */
#include <stdio.h>
#include <syscall.h>

int main ()
{
  printf("TEST: send/receive/reply\n");

  for (int i = 3; i <= 7; i++) {
    int pid = Create("test_srr_worker", 0);
    if (pid <= 0) {
      printf("Create failed\n");
      break;
    }
    int f;
    if (Send(pid, (char *)&i, sizeof(i), (char *)&f, sizeof(f)) < 0) {
      printf("Send failed\n");
      break;
    }
    printf("test...%d!=%d\n", i, f);
  }

  printf("DONE\n");
  Exit();
}

