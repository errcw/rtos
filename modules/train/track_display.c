/*
 * The display for the sensor and switch state.
 */
#include <syscall.h>
#include <terminal.h>
#include <trainlib.h>
#include "track.h"

#define SWITCH_ROW (3)
#define SWITCH_COL (65)
#define SENSOR_ROW (3)

/* Sets up and runs the program. */
static void Init ();
static void Run ();

/* Prints the state. */
static void PrintSensorState (TrackSensorState_t *se);
static void PrintSwitchState (TrackSwitchState_t *sw);

int main ()
{
  Init();
  Run();
  Exit();
}

static void Init ()
{
  /* switches */
  TerminalSetColours(TERMINAL_FG_GREEN, TERMINAL_BG_BLACK);
  TerminalRCPrintf(SWITCH_ROW, SWITCH_COL, "SWITCHES");
  TerminalSetColours(TERMINAL_FG_WHITE, TERMINAL_BG_BLACK);

  TerminalRCPrintf(SWITCH_ROW+1, SWITCH_COL, "  1:     2:");
  TerminalRCPrintf(SWITCH_ROW+2, SWITCH_COL, "  3:     4:");
  TerminalRCPrintf(SWITCH_ROW+3, SWITCH_COL, "  5:     6:");
  TerminalRCPrintf(SWITCH_ROW+4, SWITCH_COL, "  7:     8:");
  TerminalRCPrintf(SWITCH_ROW+5, SWITCH_COL, "  9:    10:");
  TerminalRCPrintf(SWITCH_ROW+6, SWITCH_COL, " 11:    12:");
  TerminalRCPrintf(SWITCH_ROW+7, SWITCH_COL, " 13:    14:");
  TerminalRCPrintf(SWITCH_ROW+8, SWITCH_COL, " 15:    16:");
  TerminalRCPrintf(SWITCH_ROW+9, SWITCH_COL, " 17:    18:");
  TerminalRCPrintf(SWITCH_ROW+10, SWITCH_COL, "153:   154:");
  TerminalRCPrintf(SWITCH_ROW+11, SWITCH_COL, "155:   156:");

  /* sensors */
  TerminalSetColours(TERMINAL_FG_GREEN, TERMINAL_BG_BLACK);
  TerminalRCPrintf(SENSOR_ROW, 1, "SENSORS");
  TerminalSetColours(TERMINAL_FG_WHITE, TERMINAL_BG_BLACK);

  TerminalRCPrintf(SENSOR_ROW, 8, "    A         B         C         D         E");
  for (int x = 0; x < SENSOR_PER_MOD; x++) {
    TerminalRCPrintf(SENSOR_ROW+x+1, 1, "%d", x+1);
  }

  /* observe the track */
  TrackCommand_t obs_cmd;
  obs_cmd.type = TRACK_OBSERVER;
  obs_cmd.c.obs.pid = MyPid();
  Send(WhoIs("track"), (char *)&obs_cmd, sizeof(obs_cmd), 0, 0);
}

static void Run ()
{
  TrackCommand_t command;
  for (;;) {
    /* receive an update */
    int pid = Receive((char *)&command, sizeof(command));
    Reply(pid, 0, 0);

    /* update the display */
    switch (command.type) {
      case TRACK_SENSOR: PrintSensorState(&command.c.sens); break;
      case TRACK_SWITCH: PrintSwitchState(&command.c.sw); break;
      default: break;
    }
  }
}

static void PrintSwitchState (TrackSwitchState_t *sw)
{
  int swnum = sw->swnum;
  char state = sw->direction == SWITCH_STRAIGHT ? 'S' : 'C';

  int col_offset = (swnum % 2) ? 5 : 12;
  int row_offset;
  if (swnum <= 18) {
    row_offset = (swnum+1)/2;
  } else {
    row_offset = (swnum-153)/2 + 10;
  }

  TerminalRCPrintf(SWITCH_ROW+row_offset, SWITCH_COL+col_offset, "%c", state);
}

static void PrintSensorState (TrackSensorState_t *se)
{
  Time_t time = GetTime();

  for (int mod = 0; mod < SENSOR_MODS_MAX; mod++) {
    unsigned int lowsensor = se->modules[mod].lo;
    unsigned int highsensor = se->modules[mod].hi;

    for (int x = 8; x > 0; x--) {
      if (lowsensor & 1) {
        TerminalRCPrintf(SENSOR_ROW+x, 10*(mod+1), "%2d:%02d.%d", time.min, time.sec, time.ms);
      }
      lowsensor >>= 1;
    }

    for (int x = 8; x > 0; x--) {
      if (highsensor & 1) {
        TerminalRCPrintf(SENSOR_ROW+8+x, 10*(mod+1), "%2d:%02d.%d", time.min, time.sec, time.ms);
      }
      highsensor >>= 1;
    }

  }
}

