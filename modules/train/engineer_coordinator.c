/*
 * Coordinates multiple engineers on the track.
 */
#include <stdio.h>
#include <string.h>

#include <syscall.h>
#include <trainlib.h>
#include "track.h"
#include "engineer.h"

/* The command event queue. */
#define CMDQ_MAX (100)
static struct {
  EngineerCommand_t cmds[CMDQ_MAX];
  unsigned int head;
  unsigned int tail;
  unsigned int size;
} cmdq;

/* The track manager. */
static int track_pid;

/* Track reservations. */
static int track_reservations[SENSORS_MAX];

/* Set up and run the program. */
static void Init ();
static void Run ();

/* Courier to engineers. */
static void QueueCommand (EngineerCommand_t *cmd);
static void SendCommand (int courier_pid);
static int courier_pid;

/* Track state updates. */
static void SwitchUpdate (TrackSwitchState_t *sw);
static void SensorUpdate (TrackSensorState_t *se);

/* Reserves a section of track. */
static void ReserveTrack (int eng_pid, EngineerReservation_t *res);


int main ()
{
  Init();
  Run();
  Exit();
}

static void Init ()
{
  RegisterAs("coordinator");

  cmdq.head = 0;
  cmdq.tail = 0;
  cmdq.size = 0;
  courier_pid = 0;

  memset(track_reservations, 0, SENSORS_MAX * sizeof(int));

  track_pid = WhoIs("track");

  TrackCommand_t obs_cmd;
  obs_cmd.type = TRACK_OBSERVER;
  obs_cmd.c.obs.pid = MyPid();
  Send(track_pid, (char *)&obs_cmd, sizeof(obs_cmd), 0, 0);
}

static void Run ()
{
  /* the command */
  union {
    int type;
    EngineerCommand_t ecmd;
    TrackCommand_t tcmd;
  } cmd;

  /* process commands */
  for (;;) {
    int pid = Receive((char *)&cmd, sizeof(cmd));
    switch (cmd.type) {
      case TRAIN_REGISTER:
      case TRAIN_POSITION_LOST:
        Reply(pid, 0, 0);
        QueueCommand(&cmd.ecmd);
        break;

      case TRAIN_RESERVE:
        ReserveTrack(pid, &cmd.ecmd.c.res);
        break;

      case TRACK_SENSOR:
        Reply(pid, 0, 0);
        SensorUpdate(&cmd.tcmd.c.sens);
        break;
      case TRACK_SWITCH:
        Reply(pid, 0, 0);
        SwitchUpdate(&cmd.tcmd.c.sw);
        break;

      case TRAIN_COURIER:
        SendCommand(pid);
        break;
    }
  }
}

static void SwitchUpdate (TrackSwitchState_t *sw)
{
  EngineerCommand_t e_cmd;
  e_cmd.type = TRAIN_TRACK_SWITCH;
  e_cmd.c.sw.sw = sw->swnum;
  e_cmd.c.sw.state = sw->direction;

  QueueCommand(&e_cmd);
}

static void SensorUpdate (TrackSensorState_t *se)
{
  EngineerCommand_t e_cmd;
  e_cmd.type = TRAIN_TRACK_SENSOR;

  for (int mod = 0; mod < SENSOR_MODS_MAX; mod++) {
    for (int i = 0; i < 8; i++) {
      if ((se->modules[mod].lo >> i) & 1) {
        e_cmd.c.se.sensor = mod*SENSOR_PER_MOD + (7 - i);
        QueueCommand(&e_cmd);
      }
    }
    for (int i = 0; i < 8; i++) {
      if ((se->modules[mod].hi >> i) & 1) {
        e_cmd.c.se.sensor = mod*SENSOR_PER_MOD + 8 + (7 - i);
        QueueCommand(&e_cmd);
      }
    }
  }
}

static void ReserveTrack (int eng_pid, EngineerReservation_t *res)
{
  /* free the previous reservations */
  for (int i = 0; i < SENSORS_MAX; i++) {
    if (track_reservations[i] == eng_pid) {
      track_reservations[i] = 0;
    }
  }

  /* create a new set of reservations */
  EngineerReservationResponse_t response = { RESERVATION_OK };

  TrackCommand_t tcmd;
  tcmd.type = TRACK_GETNEXT_SENSORS;
  tcmd.c.sq.start = res->start;
  tcmd.c.sq.distance = res->distance;
  TrackSensorQueryReply_t sr;

  Send(track_pid, (char *)&tcmd, sizeof(tcmd), (char *)&sr, sizeof(sr));

  if (!track_reservations[res->start]) {
    track_reservations[res->start] = eng_pid;
  }

  for (int i = 0; i < sr.size; i++) {
    int cur = sr.sensors[i];
    int ocur = TRACK_OPPOSITE_SENSOR(cur);

    if (track_reservations[cur] && track_reservations[cur] != eng_pid) {
      /* same direction collision */
      response.type = RESERVATION_DENIED;
      response.collision = COLLISION_FORWARD;
      break;

    } else if (track_reservations[ocur] && track_reservations[ocur] != eng_pid) {
      /* head-on collision */
      response.type = RESERVATION_DENIED;
      response.collision = COLLISION_HEADON;
      break;

    } else {
      /* track is free: make the reservation */
      track_reservations[cur] = eng_pid;

    }
  }

  Reply(eng_pid, (char *)&response, sizeof(response));
}


static void QueueCommand (EngineerCommand_t *cmd)
{
  if (cmdq.size < CMDQ_MAX) {
    cmdq.cmds[ cmdq.tail ] = *cmd;
    cmdq.tail = (cmdq.tail + 1) % CMDQ_MAX;
    cmdq.size = (cmdq.size + 1);
    if (courier_pid) {
      SendCommand(courier_pid);
      courier_pid = 0;
    }
  } else {
    printf("coord: command queue overflow\n");
  }
}

static void SendCommand (int courier)
{
  if (cmdq.size > 0) {
    EngineerCommand_t cmd = cmdq.cmds[ cmdq.head ];
    cmdq.head = (cmdq.head + 1) % CMDQ_MAX;
    cmdq.size = (cmdq.size - 1);
    Reply(courier, (char *)&cmd, sizeof(cmd));
  } else {
    courier_pid = courier;
  }
}

