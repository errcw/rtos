/*
 * Manages sending things to engineers.
 */
#include <stdio.h>

#include <syscall.h>
#include <trainlib.h>
#include "engineer.h"

#define DISTANCE_MAX (100000.0)

/* The engineers we are communicating with. */
static EngineerId_t engineers[TRAINS_MAX];
static int engineers_size;

/* A lost engineer. */
static int lost_engineers[TRAINS_MAX];
static int lost_engineers_size;

/* Registers an engineer with this courier. */
static void AddEngineer (EngineerId_t *id);

/* Indicates a lost train. */
static void TrainLost (EngineerId_t *id);

/* Sends commands to the engineers. */
static void SendCommand (EngineerCommand_t *cmd, int eng_pid);
static void BroadcastCommand (EngineerCommand_t *cmd);
static void BroadcastSensorCommand (EngineerCommand_t *cmd);


int main ()
{
  engineers_size = 0;
  lost_engineers_size = 0;

  int coord_pid = WhoIs("coordinator");

  EngineerCommand_t req_cmd = { TRAIN_COURIER, };
  EngineerCommand_t fwd_cmd;

  for (;;) {
    Send(coord_pid,
      (char *)&req_cmd, sizeof(req_cmd),
      (char *)&fwd_cmd, sizeof(fwd_cmd));
    switch (fwd_cmd.type) {
      case TRAIN_REGISTER:
        AddEngineer(&fwd_cmd.c.id);
        break;
      case TRAIN_TRACK_SENSOR:
        BroadcastSensorCommand(&fwd_cmd);
        break;
      case TRAIN_TRACK_SWITCH:
        BroadcastCommand(&fwd_cmd);
        break;
      case TRAIN_POSITION_LOST:
        TrainLost(&fwd_cmd.c.id);
        break;
      default:
        printf("ccord courier: unknown command %d\n", fwd_cmd.type);
        break;
    }
  }

  Exit();
}

static void AddEngineer (EngineerId_t *id)
{
  engineers[engineers_size] = *id;
  engineers_size += 1;
}

static void TrainLost (EngineerId_t *id)
{
  lost_engineers[lost_engineers_size] = id->pid;
  lost_engineers_size += 1;
}

static void SendCommand (EngineerCommand_t *cmd, int eng_pid)
{
  Send(eng_pid, (char *)cmd, sizeof(EngineerCommand_t), 0, 0);
}

static void BroadcastCommand (EngineerCommand_t *cmd)
{
  for (int e = 0; e < engineers_size; e++) {
    SendCommand(cmd, engineers[e].pid);
  }
}

static void BroadcastSensorCommand (EngineerCommand_t *cmd)
{
  int best_pid = 0;
  EngineerSensorEval_t best_eval = { SENSOR_UNEXPECTED, DISTANCE_MAX, 0 };

  for (int e = 0; e < engineers_size; e++) {
    EngineerSensorEval_t cur_eval;
    Send(engineers[e].pid,
      (char *)cmd, sizeof(EngineerCommand_t),
      (char *)&cur_eval, sizeof(cur_eval));

    if ((cur_eval.type == SENSOR_EXPECTED && best_eval.type != SENSOR_EXPECTED) ||
        (cur_eval.type <= best_eval.type && cur_eval.distance < best_eval.distance)) {
      best_pid = engineers[e].pid;
      best_eval = cur_eval;
    }
  }

  int got_pid = 0;
  if (best_pid && best_eval.type != SENSOR_UNEXPECTED && best_eval.inside_window) {
    got_pid = best_pid;
  } else if (lost_engineers_size && best_eval.type == SENSOR_UNEXPECTED) {
    lost_engineers_size -= 1;
    got_pid = lost_engineers[lost_engineers_size];
  }
  if (got_pid) {
    cmd->type = TRAIN_GOTSENSOR;
    Send(got_pid, (char *)cmd, sizeof(EngineerCommand_t), 0, 0);
  }
}

