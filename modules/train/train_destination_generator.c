#include <syscall.h>
#include <stdlib.h>
#include "track.h"
#include "heartbeat.h"
#include "engineer.h"

#define MAX_WAIT_TIME (5)
#define POLL_DELAY (1000)

/* The engineer for which we are generating destinations. */
static int eng_pid;

/* Generate a random destination. */
static int GetRandomSensor ();
static int GetRandomSpeed ();
static int GetRandomOffset ();

/* Filter bad/dangerous places on the track. */
static int IsSensorAcceptable (int sensor);

/* Control the engineer. */
static void SetDestination (int sensor, int offset, int speed);
static int IsRepathTime ();

int main ()
{
  int creator = Receive((char *)&eng_pid, sizeof(eng_pid));
  Reply(creator, 0, 0);

  for (;;) {
    if (IsRepathTime()) {
      SetDestination(GetRandomSensor(), GetRandomOffset(), GetRandomSpeed());
    }
    Delay(POLL_DELAY);
  }

  Exit();
}

static int GetRandomSensor ()
{
  int dest_sensor;
  do {
    dest_sensor = (rand() % SENSORS_MAX) + 1;
  } while (!IsSensorAcceptable(dest_sensor));
  return dest_sensor;
}

static int GetRandomSpeed ()
{
  return 8;
}

static int GetRandomOffset ()
{
  return 0;
}

static void SetDestination (int sensor, int offset, int speed)
{
  EngineerCommand_t cmd;
  cmd.type = TRAIN_DESTINATION;
  cmd.c.dest.speed = speed;
  cmd.c.dest.offset = offset;
  cmd.c.dest.type = NODE_SENSOR;
  cmd.c.dest.id = sensor;

  Send(eng_pid, (char *)&cmd, sizeof(cmd), 0, 0);
}

static int IsRepathTime ()
{
  static int timeout = 0;

  EngineerCommand_t cmd = { TRAIN_POSITION, };
  TrainPosition_t pos;

  Send(eng_pid, (char *)&cmd, sizeof(cmd), (char *)&pos, sizeof(pos)); 

  if (pos.journey_valid && pos.speed == 0) {
    timeout += 1;
  } else if (pos.speed > 0) {
    timeout = 0;
  }

  if (timeout >= MAX_WAIT_TIME || !pos.journey_valid) {
    timeout = 0;
    return 1;
  }
  return 0;
}

static int IsSensorAcceptable (int sensor)
{
  return (sensor < 22 || sensor > 27);
}

