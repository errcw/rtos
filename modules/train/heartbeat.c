#include <syscall.h>
#include "heartbeat.h"

int main ()
{
  HeartbeatCfg_t cfg;

  int creator = Receive((char *)&cfg, sizeof(cfg));
  Reply(creator, 0, 0);

  const int heartbeat = HEARTBEAT;

  for (;;) {
    Delay(cfg.delay);
    Send(cfg.pid, (char *)&heartbeat, sizeof(heartbeat), 0, 0);
  }

  Exit();
}

