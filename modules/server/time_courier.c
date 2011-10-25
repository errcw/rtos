/*
 * Time server messenger.
 */
#include <timeserver.h>
#include <syscall.h>

int main()
{
  const int tick_msg = TIMESERVER_TICK;

  int server_pid = Receive(0, 0);
  Reply(server_pid, 0, 0);

  for (;;) {
    /* receive a tick from the notifier */
    int notifier_pid = Receive(0, 0);
    Reply(notifier_pid, 0, 0);
    /* send the tick to the server */
    Send(server_pid, (char *)&tick_msg, sizeof(tick_msg), 0, 0);
  }

  Exit();
}

