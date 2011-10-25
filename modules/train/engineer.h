/*
 * A train engineer.
 */
#ifndef __ENGINEER_H
#define __ENGINEER_H

/* Specifies the engineer id. */
typedef struct EngineerId {
  int pid;
  int train;
} EngineerId_t;

/* Sets the forward speed. */
typedef struct EngineerForward {
  int speed;
} EngineerForward_t;

/* Routes to a destination. */
typedef struct EngineerDestination {
  int type; /* NODE_{SENSOR, SWITCH, STOP} */
  int id;
  int sw_state;
  int offset;
  int speed;
} EngineerDestination_t;

/* Indicates a sensor fired. */
typedef struct EngineerSensor {
  int sensor;
} EngineerSensor_t;

/* Evaluation of a sensor that fired. */
typedef struct EngineerSensorEval {
  enum {
    SENSOR_EXPECTED = 0,
    SENSOR_POTENTIAL = 1,
    SENSOR_UNEXPECTED = 2
  } type;
  float distance;
  int inside_window;
} EngineerSensorEval_t;

/* Indicates a switch changed state. */
typedef struct EngineerSwitch {
  int sw;
  int state;
} EngineerSwitch_t;

/* A reservation for a piece of track. */
typedef struct EngineerReservation {
  int start;
  float distance;
} EngineerReservation_t;

/* Response to a reservation attempt. */
typedef struct EngineerReservationResponse {
  enum {
    RESERVATION_OK,
    RESERVATION_DENIED
  } type;
  enum {
    COLLISION_FORWARD,
    COLLISION_HEADON
  } collision;
} EngineerReservationResponse_t;

/* An engineer command. */
typedef struct EngineerCommand {
  enum {
    TRAIN_REGISTER = 0x80,

    TRAIN_INITIALIZE,
    TRAIN_FORWARD,
    TRAIN_REVERSE,
    TRAIN_DESTINATION,

    TRAIN_POSITION,
    TRAIN_POSITION_LOST,

    TRAIN_TRACK_SENSOR,
    TRAIN_TRACK_SWITCH,
    TRAIN_GOTSENSOR,

    TRAIN_RESERVE,

    TRAIN_COURIER
  } type;
  union {
    EngineerId_t id;
    EngineerForward_t fwd;
    EngineerDestination_t dest;
    EngineerSensor_t se;
    EngineerSwitch_t sw;
    EngineerReservation_t res;
  } c;
} EngineerCommand_t;

/* Configuration for the train tracking display. */
typedef struct TrainDisplayCfg {
  int row;
  int train;
  int engineer_pid;
} TrainDisplayCfg_t;

/* The position of the train on the track. */
typedef struct TrainPosition {
  /* train position */
  int position;
  int position_type;
  int distance;
  int switch_state;
  /* train speed */
  int speed;
  /* the next sensor */
  int sensor;
  int sensor_dist;
  Time_t sensor_time;
  int sensor_valid;
  /* journey */
  int journey_next;
  int journey_valid;
} TrainPosition_t;

#endif

