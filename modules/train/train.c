/*
 * Creates the train processes and gathers user input.
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <syscall.h>
#include <terminal.h>
#include <trainlib.h>
#include "track.h"
#include "engineer.h"
#include "courier.h"

/* Maximum number of characters in a command. */
#define MAX_CMD_LEN (32)

/* The starting positions of the command line. */
#define CMD_R (1)
#define CMD_C (4)

/* The track manager pid. */
static int track_pid;

/* The tracker pids. */
static int engineer_pids[TRAINS_MAX];
static int train_display_pids[TRAINS_MAX];
static int randpath_pids[TRAINS_MAX];

/* The track position. */
static int tracker_pos;

/* Set up and run the program. */
static void Init ();
static void Run ();

/* Gets the track on which to run. */
static void SetupTrack ();

/* Parses a command. */
static void ParseCommand (char *command);

/* Sends a command to an engineer. */
static void CreateEngineer (int train);
static void CreateEngineerDisplay (int train);
static void SendToEngineer (int train, EngineerCommand_t *cmd);

/* Prints command line things. */
static void PrintPrompt ();
static void PrintWarning (char *warning);
static void ClearWarning ();

static void ParsePathCommand(int train, int speed, char *dest);
static void ParseBreakCommand (char *str_start);
static void SetupRandomDestinations (int train);

/* Exit permanently. */
static void Quit();


int main ()
{
  Init();
  Run();
  Exit();
}

static void Init ()
{
  TrainInit();

  TerminalInit();
  TerminalClear();
  tracker_pos = 21;
   
  memset(engineer_pids, 0, TRAINS_MAX * sizeof(int));
  memset(train_display_pids, 0, TRAINS_MAX * sizeof(int));
  memset(randpath_pids, 0, TRAINS_MAX * sizeof(int));

  int pri = MyPriority();

  /* create the track manager */
  track_pid = Create("track", pri+4);
  Create("track_courier", pri+4);

  /* create the sensors */
  Create("sensors", pri+5);
  Create("sensors_courier", pri+5);

  /* create the engineer coordinator */
  Create("engineer_coordinator", pri+3);
  Create("engineer_coordinator_courier", pri+3);

  /* create the display */
  Create("track_display", pri-1);
  Create("time_writer", pri-1);
}

static void Run() 
{
  SetupTrack();
  PrintPrompt();
  for (;;) {
    char *cmd = TerminalReadLine(CMD_R, CMD_C);
    ParseCommand(cmd);
  }
}

static void ParseCommand (char *command)
{
  ClearWarning();

  char action[MAX_CMD_LEN];
  if (sscanf(command, "%s", action)) {
    if (strcmp("tr", action) == 0) {
      int train, speed;
      if (sscanf(command, "%*s %d %d", &train, &speed)) {
        if (train < 0 || train > TRAINS_MAX) {
          PrintWarning("That train number doesn't make any sense.");
          return;
        }
        if (speed < TRAIN_SPEED_MIN || speed > TRAIN_SPEED_MAX) {
          PrintWarning("That train speed is impossible.");
          return;
        }

        char dest[MAX_CMD_LEN];
				memset(dest, 0, MAX_CMD_LEN);
        if (sscanf(command, "%*s %*d %*d %s", dest)) {
          if (dest[0] != '\0') {
            /* parse the position */
            ParsePathCommand(train, speed, dest); 
          } else {
            /* set the speed */
            EngineerCommand_t cmd;
            cmd.type = TRAIN_FORWARD;
            cmd.c.fwd.speed = speed;
            SendToEngineer(train, &cmd);
          }
				}
        return;
      }

    } else if (strcmp("rev", action) == 0) {
      int train;
      if (sscanf(command, "%*s %d", &train)) {
        if (train < 0 || train > TRAINS_MAX) {
          PrintWarning("That train number doesn't make any sense.");
          return;
        }
        /* reverse the train */
        EngineerCommand_t cmd = { TRAIN_REVERSE, };
        SendToEngineer(train, &cmd);
        return;
      }

    } else if (strcmp("sw", action) == 0) {
      int swnum;
      if (sscanf(command, "%*s %d %s", &swnum, action)) {
        if (swnum < 0 || swnum > SWITCHES_MAX) {
          PrintWarning("That switch doesn't exist.");
          return;
        }
        int straight = (strcmp("S", action) == 0);
        int curved = (strcmp("C", action) == 0);
        if (!straight && !curved) {
          PrintWarning("That direction doesn't make any sense.");
          return;
        }
        /* send a switch message */
        TrackCommand_t trcmd;
        trcmd.type = TRACK_SWITCH;
        trcmd.c.sw.swnum = swnum;
        trcmd.c.sw.direction = straight ? SWITCH_STRAIGHT : SWITCH_CURVED;
        trcmd.c.sw.exec = 1;
        Send(track_pid, (char *)&trcmd, sizeof(trcmd), 0, 0);
        return;
      }

    } else if (strcmp("init", action) == 0) {
      int train;
      if (sscanf(command, "%*s %d", &train)) {
        if (train < 0 || train > TRAINS_MAX) {
          PrintWarning("That train number doesn't make any sense.");
          return;
        }
        /* create an engineer */
        CreateEngineer(train);
        /* ask the engineer to init */
        EngineerCommand_t cmd = { TRAIN_INITIALIZE, };
        SendToEngineer(train, &cmd);
        /* create a display */
        CreateEngineerDisplay(train);
        return;
      }

    } else if (strcmp("stop", action) == 0) {
      TrainCommand_t cmd = { TRAIN_CMD_STOP, };
      TrainExecCmd(&cmd);
      return;

    } else if (strcmp("go", action) == 0) {
      TrainCommand_t cmd = { TRAIN_CMD_GO, };
      TrainExecCmd(&cmd);
      return;

    } else if (strcmp("q", action) == 0) {
      Quit();
      return; /* should never reach here! */

    } else if (strcmp("break", action) == 0) {
      char start[MAX_CMD_LEN];
			memset(start, 0, MAX_CMD_LEN);
      if (sscanf(command, "%*s %s", start) && start[0] != '\0') {
        /* parse the position */
        ParseBreakCommand(start); 
			}
      return;

    } else if (strcmp("rand", action) == 0) {
      int train;
      if (sscanf(command, "%*s %d", &train)) {
        if (train < 0 || train > TRAINS_MAX) {
          PrintWarning("That train number doesn't make any sense.");
          return;
        }
        SetupRandomDestinations(train);
        return;
      }
    }
  }

  /* fall through to a generic error */
  PrintWarning("Oh no! I couldn't understand your command.");
}

static void SetupTrack ()
{
  TerminalSetColours(TERMINAL_FG_RED, TERMINAL_BG_BLACK);
  TerminalRCPrintf(1, 6, ">>");
  TerminalSetColours(TERMINAL_FG_WHITE, TERMINAL_BG_BLACK);
  TerminalRCPrintf(1, 1, "track");
  
  char tr;
  do {
    tr = TerminalRead();
  } while (tr != 'a' && tr != 'b');

  TrackCommand_t trcmd;
  trcmd.type = TRACK_CONFIG;
  trcmd.c.cfg.type = toupper(tr);
  Send(track_pid, (char *)&trcmd, sizeof(trcmd), 0, 0);
}

static void CreateEngineer (int train)
{
  if (!ValidPid(engineer_pids[train])) {
    int engineer_pid = Create("engineer", MyPriority()+1);
    engineer_pids[train] = engineer_pid;

    Send(engineer_pid, (char *)&train, sizeof(train), 0, 0);
  }
}

static void SendToEngineer (int train, EngineerCommand_t *cmd)
{
  if (!ValidPid(engineer_pids[train])) {
    PrintWarning("That train is not initialized.");
  }
  /* send it the command */
  Send(engineer_pids[train], (char *)cmd, sizeof(EngineerCommand_t), 0, 0);
}

static void CreateEngineerDisplay (int train)
{
  if (!ValidPid(train_display_pids[train])) {
    int display_pid = Create("train_display", MyPriority());
    train_display_pids[train] = display_pid;

    TrainDisplayCfg_t disp_cfg;
    disp_cfg.row = tracker_pos++;
    disp_cfg.train = train;
    disp_cfg.engineer_pid = engineer_pids[train];

    Send(display_pid, (char *)&disp_cfg, sizeof(disp_cfg), 0, 0);
  }
}

static void PrintPrompt ()
{
  TerminalSetColours(TERMINAL_FG_RED, TERMINAL_BG_BLACK);
  TerminalRCPrintf(1, 1, ">>       ");
  TerminalSetColours(TERMINAL_FG_WHITE, TERMINAL_BG_BLACK);
}

static void PrintWarning (char *warning)
{
  TerminalSetColours(TERMINAL_FG_RED, TERMINAL_BG_WHITE);
  TerminalRCPrintf(2, 1, "%s", warning);
  TerminalSetColours(TERMINAL_FG_WHITE, TERMINAL_BG_BLACK);
}

static void ClearWarning ()
{
  TerminalRCPrintf(2, 1 ,"%80s", " ");
}

static void ParsePathCommand (int train, int speed, char *str_dest)
{
  EngineerCommand_t cmd;
  cmd.type = TRAIN_DESTINATION;
  EngineerDestination_t *dest = &cmd.c.dest;
  dest->speed = speed;
  dest->offset = 0;
  
  if (isalpha(str_dest[0]) && isdigit(str_dest[1])) {
    dest->type = NODE_SENSOR;

		char mod;
		int sensor;
    sscanf(str_dest, "%c%d+%d", &mod, &sensor, &(dest->offset));
		if (sensor < 1 || sensor > SENSOR_PER_MOD) {
      PrintWarning("That sensor id doesn't make sense.");
			return;
		}
    switch (mod) {
      case 'A': cmd.c.dest.id = 0 * SENSOR_PER_MOD + (sensor - 1); break;
      case 'B': cmd.c.dest.id = 1 * SENSOR_PER_MOD + (sensor - 1); break;
      case 'C': cmd.c.dest.id = 2 * SENSOR_PER_MOD + (sensor - 1); break;
      case 'D': cmd.c.dest.id = 3 * SENSOR_PER_MOD + (sensor - 1); break;
      case 'E': cmd.c.dest.id = 4 * SENSOR_PER_MOD + (sensor - 1); break;
			default:
				PrintWarning("That sensor module doesn't make sense.");
				return;
    }

  } else {
    if (isdigit(str_dest[0])) {
      dest->type = NODE_SWITCH;

			char sw_state;
      sscanf(str_dest, "%d%c+%d", &(dest->id), &sw_state, &(dest->offset));
      switch (sw_state) {
        case 'S': dest->sw_state = SWITCH_STRAIGHT; break;
        case 'C': dest->sw_state = SWITCH_CURVED; break;
        default:
          PrintWarning("That switch direction doesn't make sense."); 
          return;
			}

		} else {
      dest->type = NODE_STOP;

      sscanf(str_dest, "%*c%*c%d+%d", &(dest->id), &(dest->offset));
      if (dest->id < 1 || dest->id > STOPS_MAX) {
        PrintWarning("That stop number doesn't make sense."); 
        return;
      }
      dest->id -= 1;

		}
  }

  dest->offset *= 10; /* convert cm to mm */
  
  SendToEngineer(train, &cmd);
}

static void ParseBreakCommand (char *str_start)
{
  TrackCommand_t pmcmd;
  pmcmd.type = TRACK_MODPATH;
  pmcmd.c.pm.disconnect = 1;
  
  if (isalpha(str_start[0]) && isdigit(str_start[1])) {
		char mod;
		int sensor;
    sscanf(str_start, "%c%d", &mod, &sensor);
		if (sensor < 1 || sensor > SENSOR_PER_MOD) {
      PrintWarning("That sensor id doesn't make sense.");
			return;
		}
    int start_sensor;
    switch (mod) {
      case 'A': start_sensor = 0 * SENSOR_PER_MOD + (sensor - 1); break;
      case 'B': start_sensor = 1 * SENSOR_PER_MOD + (sensor - 1); break;
      case 'C': start_sensor = 2 * SENSOR_PER_MOD + (sensor - 1); break;
      case 'D': start_sensor = 3 * SENSOR_PER_MOD + (sensor - 1); break;
      case 'E': start_sensor = 4 * SENSOR_PER_MOD + (sensor - 1); break;
			default:
				PrintWarning("That sensor module doesn't make sense.");
				return;
    }

    pmcmd.c.pm.sensor = start_sensor;

  } else {
	  PrintWarning("This function only works on sensors.");
    return;
  }

  Send(track_pid, (char *)&pmcmd, sizeof(pmcmd), 0, 0);
}

static void SetupRandomDestinations (int train)
{
  if (randpath_pids[train]) {
    Destroy(randpath_pids[train]);
    randpath_pids[train] = 0;

    EngineerCommand_t cmd;
    cmd.type = TRAIN_FORWARD;
    cmd.c.fwd.speed = 0;
    SendToEngineer(train, &cmd);

  } else {
    int eng_pid = engineer_pids[train];
    if (eng_pid <= 0) {
      PrintWarning("That train is not initialized.");
      return;
    }
    randpath_pids[train] = Create("train_destination_generator", MyPriority() - 1);
    Send(randpath_pids[train], (char *)&eng_pid, sizeof(eng_pid), 0, 0);

  }
}

static void Quit ()
{
  /* destroy the engineers */
  for (int e = 0; e < TRAINS_MAX; e++) {
    if (!engineer_pids[e]) {
      continue;
    }

    EngineerCommand_t cmd;
    cmd.type = TRAIN_FORWARD;
    cmd.c.fwd.speed = 0;
    SendToEngineer(e, &cmd);

    Destroy(randpath_pids[e]);
    Destroy(train_display_pids[e]);
    Destroy(engineer_pids[e]);
  }

  /* destroy the track manager */
  Destroy(WhoIs("track"));

  /* destroy the time writer */
  Destroy(WhoIs("time_writer"));

  TerminalSetColours(TERMINAL_FG_RED, TERMINAL_BG_WHITE);
  TerminalRCPrintf(1, 1, "Dave, my mind is going. I can feel it. I can feel it.");
  TerminalRCPrintf(2, 1, "My mind is going. There is no question about it. I can feel it.");
  TerminalRCPrintf(3, 1, "My mind is going. There is no question about it. I can feel it.");
  TerminalRCPrintf(4, 1, "I'm a...fraid.");

  TerminalSetColours(TERMINAL_FG_GREEN, TERMINAL_BG_WHITE);
  TerminalRCPrintf(20, 1, "Daisy, Daisy, give me your answer do.");
  TerminalRCPrintf(21, 1, "I'm half crazy all for the love of you.");

  /* stick a fork in ourselves */
  Exit();
}

