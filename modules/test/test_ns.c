/*
 * Name server test.
 */
#include <stdio.h>
#include <syscall.h>
#include <error.h>

static void TestSelf();
static void TestChildren ();

int main ()
{
  printf("TEST: name server\n");

  TestSelf();
  TestChildren();

  printf("DONE\n");

  Exit();
}

static void TestSelf ()
{
  /* find our pid */
  int pid = MyPid();
  if (pid <= 0) {
    printf("MyPid failed");
    return;
  }

  /* register ourselves */
  if (RegisterAs("nstest") < 0) {
    printf("RegisterAs failed\n");
    return;
  }

  /* look ourselves up */
  int wpid = WhoIs("nstest");
  if (wpid <= 0) {
    printf("WhoIs failed\n");
    return;
  }

  /* check we got the right pid */
  printf("test...MyPid=%d, WhoIs=%d\n", pid, wpid);

  /* register oursevles under another name */
  if (RegisterAs("nstest-2") < 0) {
    printf("RegisterAs failed\n");
    return;
  }

  /* look ourselves up again */
  int wpid2 = WhoIs("nstest-2");
  if (wpid2 <= 0) {
    printf("WhoIs failed\n");
    return;
  }

  /* check we got the right pid again */
  printf("test...MyPid=%d, WhoIs=%d\n", pid, wpid2);
}

static void TestChildren ()
{
  char cname[128];

  for (int i = 0; i < 3; i++) {
    int cpid = Create("test_ns_child", MyPriority() + 1);
    if (cpid <= 0) {
      printf("Create failed\n");
      return;
    }
    sprintf(cname, "ns-child-%d", cpid);
    int wcpid = WhoIs(cname);
    if (wcpid <= 0) {
      printf("WhoIs failed\n");
    }
    printf("test...Create=%d, WhoIs=%d\n", cpid, wcpid);
  }
}

