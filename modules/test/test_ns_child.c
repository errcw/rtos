/*
 * Name server test (child processes).
 */
#include <stdio.h>
#include <syscall.h>

int main ()
{
  /* look up our parent */
  int parent = MyParentPid();
  if (parent <= 0) {
    printf("MyParentPid failed\n");
    Exit();
  }
  int wparent = WhoIs("nstest");
  if (wparent <= 0) {
    printf("WhoIs failed\n");
    Exit();
  }
  if (parent != wparent) {
    printf("MyParentPid and WhoIs do not match (%d and %d)\n", parent, wparent);
    Exit();
  }

  /* register ourselves */
  int pid = MyPid();
  if (pid <= 0) {
    printf("MyPid failed\n");
    Exit();
  }
  char name[128];
  sprintf(name, "ns-child-%d", pid);
  if (RegisterAs(name) < 0) {
    printf("RegisterAs failed\n");
  }
  Exit();
}

