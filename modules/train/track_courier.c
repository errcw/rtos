/*
 * A track courier
 */
#include <stdio.h>
#include <syscall.h>
#include <limits.h>
#include "track.h"
#include "courier.h"

/* Our observers. */
static int observers[PROCESS_MAX];
static int observers_size;

/* Handles a command from the track. */
static void HandleCommand (TrackCommand_t *cmd);

int main ()
{
  observers_size = 0;

  int track_pid = WhoIs("track");

  TrackCommand_t req_cmd = { TRACK_COURIER, };
  TrackCommand_t fwd_cmd;

  for (;;) {
    Send(track_pid, (char *)&req_cmd, sizeof(req_cmd), (char *)&fwd_cmd, sizeof(fwd_cmd));
    HandleCommand(&fwd_cmd);
  }

  Exit();
}

static void HandleCommand (TrackCommand_t *cmd)
{
  if (cmd->type == TRACK_OBSERVER) {
    /* add another observer */
    observers[observers_size] = cmd->c.obs.pid;
    observers_size += 1;
  } else {
    /* forward the command to all the observers */
    for (int o = 0; o < observers_size; o++) {
      Send(observers[o], (char *)cmd, sizeof(TrackCommand_t), 0, 0);
    }
  }
}

