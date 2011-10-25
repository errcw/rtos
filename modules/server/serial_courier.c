/*
 * Serial server courier.
 */
#include <serialserver.h>
#include <syscall.h>

int main ()
{
  SerialCourierCfg_t cfg;
  SerialMsg_t msg;

  /* receive our initial configuration */
  int server_pid = Receive((char *)&cfg, sizeof(cfg));
  Reply(server_pid, 0, 0);

  for (;;) {
    /* request the data from the source */
    Send(cfg.src_pid, (char *)&cfg.cmd, sizeof(cfg.cmd), (char *)&msg, sizeof(msg));
    /* now forward it to the destination */
    Send(cfg.dst_pid, (char *)&msg, msg.len+2, 0, 0);
  }

  Exit();
}

