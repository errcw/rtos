/*
 * A train display.
 */
#include <stdio.h>
#include <syscall.h>
#include <terminal.h>
#include <trainlib.h>
#include "engineer.h"
#include "track.h"

#define TRAIN_DISPLAY_DELAY (75)

/* The train display configuration. */
static TrainDisplayCfg_t cfg;

/* Set up and run the program. */
static void Init ();
static void Run ();

/* Prints the trains position. */
static void PrintPosition (TrainPosition_t *pos);

/* Gets a human-readable names for track positions. */
static void GetSensorName (int sensor, char *sname);
static void GetSwitchName (int sw, int state, char *swname);
static void GetDeadEndName (int de, char *dename);

int main ()
{
  Init();
  Run();
  Exit();
}

static void Init ()
{
  /* receive our configuration */
  int creator = Receive((char *)&cfg, sizeof(cfg));
  Reply(creator, 0, 0);

  /* print the format */
  TerminalRCPrintf(cfg.row, 1, "(Train %d)", cfg.train);
  TerminalRCPrintf(cfg.row, 15, "P:      +    cm", cfg.train);
  TerminalRCPrintf(cfg.row, 35, "S:    cm/s", cfg.train);
  TerminalRCPrintf(cfg.row, 55, "T:   .  s", cfg.train);
}

static void Run ()
{
  EngineerCommand_t req_cmd = { TRAIN_POSITION, };
  TrainPosition_t pos;

  for (;;) {
    Send(cfg.engineer_pid, (char *)&req_cmd, sizeof(req_cmd), (char *)&pos, sizeof(pos));
    PrintPosition(&pos);
    Delay(TRAIN_DISPLAY_DELAY);
  }
}

static void PrintPosition (TrainPosition_t *pos)
{
  static TrainPosition_t old_pos = { -1, };

  /* position */
  if (pos->position != old_pos.position) {
    char posname[8];
    if (pos->position_type == NODE_SWITCH) {
      GetSwitchName(pos->position, pos->switch_state, posname);
    } else if (pos->position_type == NODE_STOP) {
      GetDeadEndName(pos->position, posname);
    } else if (pos->position_type == NODE_SENSOR) {
      GetSensorName(pos->position, posname);
    }
    TerminalRCPrintf(cfg.row, 18, "%s", posname);
  }
  if (pos->distance != old_pos.distance) {
    if (pos->distance >= 0 && pos->distance < 1000) {
      TerminalRCPrintf(cfg.row, 25, "%3d", pos->distance);
    } else {
      TerminalRCPrintf(cfg.row, 25, "???", pos->distance);
    }
  }

  /* speed */
  if (pos->speed != old_pos.speed) {
    TerminalRCPrintf(cfg.row, 38, "%2d", pos->speed);
  }

  /* next sensor */
  if (pos->sensor_valid) {
    if (pos->sensor_time.sec != old_pos.sensor_time.sec) {
      TerminalRCPrintf(cfg.row, 58, "%2d", pos->sensor_time.sec);
    }
    if (pos->sensor_time.ms != old_pos.sensor_time.ms) {
      TerminalRCPrintf(cfg.row, 61, "%d", pos->sensor_time.ms / 100);
    }
  } else {
    if (pos->sensor_valid != old_pos.sensor_valid) {
      /* we do not know when the next sensor will occur */
      TerminalRCPrintf(cfg.row, 58, "  .  ");
    }
  }

  old_pos = *pos;
}

static void GetSwitchName (int sw, int state, char *swname)
{
  char state_char = (state == SWITCH_STRAIGHT) ? 'S' : 'C';
  sprintf(swname, "%d%c ", sw, state_char);
}

static void GetSensorName (int sensor, char *sname)
{
  char module = '?';
  switch (sensor / SENSOR_PER_MOD) {
    case 0: module = 'A'; break;
    case 1: module = 'B'; break;
    case 2: module = 'C'; break;
    case 3: module = 'D'; break;
    case 4: module = 'E'; break;
  }
  int snum = (sensor % SENSOR_PER_MOD) + 1;
  sprintf(sname, "%c%d  ", module, snum);
}

static void GetDeadEndName (int de, char *dename)
{
  sprintf(dename, "DE%d ", de + 1);
}

