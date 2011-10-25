/*
 * Sensor courier; forwards data from the sensor reader to the track.
 */
#include <syscall.h>
#include "track.h"
#include "courier.h"

int main ()
{
  int sensors_pid = WhoIs("sensors");
  int track_pid = WhoIs("track");

  TrackCommand_t track_cmd;

  for (;;) {
    /* read the sensor data */
    Send(sensors_pid, 0, 0, (char *)&track_cmd, sizeof(track_cmd));
    /* forward it to the track */
    Send(track_pid, (char *)&track_cmd, sizeof(track_cmd), 0, 0);
  }

  Exit();
}

