/*
 * This is the program to run the trains.
 */

#ifndef __TRAIN_H
#define __TRAIN_H

/* The maximum number of trains. */
#define TRAINS_MAX (80)

/* The train speed limits. */
#define TRAIN_SPEED_MIN (0)
#define TRAIN_SPEED_MAX (14)
#define TRAIN_SPEED_NUM (15)

/* The maximum number of sensors and sensor modules. */
#define SENSORS_MAX (80)
#define SENSOR_MODS_MAX (5)
#define SENSOR_PER_MOD (16)

/* The maximum number of switches. */
#define SWITCHES_MAX (192)
#define SWITCH_STRAIGHT (0)
#define SWITCH_CURVED (1)

/* The maximum number of dead ends. */
#define STOPS_MAX (10)

/* A command to the train. */
#define TRAIN_CMD_NOTHING (0)
#define TRAIN_CMD_FORWARD (1)
#define TRAIN_CMD_REVERSE (2)
#define TRAIN_CMD_SWITCH (3)
#define TRAIN_CMD_SWITCH_END (4)
#define TRAIN_CMD_SENSOR (5)
#define TRAIN_CMD_FUNCTION (6)
#define TRAIN_CMD_STOP (7)
#define TRAIN_CMD_GO (8)

#define TRAIN_FUNCTION_OFF (64)
#define TRAIN_FUNCTION_HORN (68)
#define TRAIN_FUNCTION_LIGHTS (74)

typedef struct TrainCommand {
  int type;
  int id;
  int param;
} TrainCommand_t;

/*
 * Initialize the train serial port.
 */
void TrainInit ();

/*
 * Executes a command for the train.
 */
void TrainExecCmd (TrainCommand_t *cmd);

/*
 * Reads a byte of data from the trains.
 */
unsigned int TrainRead ();

#endif

