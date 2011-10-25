/*
 * The idle process.
 */
#include <syscall.h>

int main ()
{
  for (;;) {
    Pass();
  }
  Exit();
}

