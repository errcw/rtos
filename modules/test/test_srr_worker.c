/*
 * Child test process.
 */
#include <stdio.h>
#include <syscall.h>

int main ()
{
  int i, j;
  int rpid = Receive((char *)&i, sizeof(i));
  if (i > 1) {
    int cpid = Create("test_srr_worker", 0);
    j = i - 1;
    Send(cpid, (char *)&j, sizeof(j), (char *)&j, sizeof(j));
    i = i * j;
  }
  Reply(rpid, (char *)&i, sizeof(i));
  Exit();
}
