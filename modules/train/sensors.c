/*
 * Sensor monitor.
 */
#include <stdio.h>
#include <syscall.h>
#include <trainlib.h>
#include "track.h"

int main ()
{
  RegisterAs("sensors");

  TrainCommand_t train_cmd;
  train_cmd.type = TRAIN_CMD_SENSOR;
  train_cmd.id = SENSOR_MODS_MAX;

  TrackCommand_t track_cmd;
  track_cmd.type = TRACK_SENSOR;

  TrackSensorState_t *sensor_state = &track_cmd.c.sens;

  for (;;) {
    /* query the sensors */
    TrainExecCmd(&train_cmd);
    for (int mod = 0; mod < SENSOR_MODS_MAX; mod++) {
      sensor_state->modules[mod].lo = TrainRead();
      sensor_state->modules[mod].hi = TrainRead();
    }

    /* send the sensor data */
    int courier_pid = Receive(0, 0);
    Reply(courier_pid, (char *)&track_cmd, sizeof(track_cmd));
  }

  Exit();
}

