/*
 * Bootstrap process.
 */
#include <syscall.h>
#include <nameserver.h>
#include <timeserver.h>
#include <serialserver.h>
#include <limits.h>
#include <terminal.h>

static void StartModule ();

int main ()
{
  /* create the servers */
  Create("name", NAMESERVER_PRIORITY);
  Create("time", TIMESERVER_PRIORITY);

  /* create the idle process */
  Create("idle", PRIORITY_MIN);

  /* StartModule(); */
  Create("train", PRIORITY_MIN + 1);

  Exit();
}

static void StartModule ()
{
  TerminalInit();
  TerminalClear();
  TerminalRCPrintf(1, 1, "Enter a process to start:");
  TerminalRCPrintf(2, 1, ">> ");
  for (;;) {
    char *mod = TerminalReadLine(2, 4);
    int pid = Create(mod, PRIORITY_MIN + 1);
    if (pid > 0) {
      TerminalClear();
      break;
    }
    TerminalRCPrintf(3, 1, "Invalid module!");
  }
}

