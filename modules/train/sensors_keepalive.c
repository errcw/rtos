/*
 * Monitors sensors and keeps them alive.
 */
#include <stdio.h>

#include <syscall.h>
#include "track.h"
#include "heartbeat.h"

#define SENSOR_TIMEOUT (5000)
#define MSG_MAX (256)

/* The sensors and sensors_courier pids. */
static int s_pid, c_pid;

/* Restarts sensors. */
static void RestartSensors ();


int main ()
{
  int pid = MyPid();

  /* create the timeout heartbeat */
  HeartbeatCfg_t hb_cfg = { pid, SENSOR_TIMEOUT };
  Send(Create("heartbeat", MyPriority()), (char *)&hb_cfg, sizeof(hb_cfg), 0, 0);

  /* listen for sensor data */
  TrackCommand_t obs_cmd;
  obs_cmd.type = TRACK_OBSERVER;
  obs_cmd.c.obs.pid = pid;
  Send(WhoIs("track"), (char *)&obs_cmd, sizeof(obs_cmd), 0, 0);

  int got_sensor_data = 0;
  int msg[MSG_MAX];

  for (;;) {
    int sender = Receive((char *)&msg, MSG_MAX * sizeof(int));
    Reply(sender, 0, 0);

    if (msg[0] == TRACK_SENSOR) {
      got_sensor_data = 1;

    } else if (msg[0] == HEARTBEAT) {
      if (!got_sensor_data) {
        printf("sensors keepalive: found dead sensor processes, restarting\n");
        RestartSensors();
      }
      got_sensor_data = 0;

    }
  }
  
  Exit();
}

static void RestartSensors ()
{
  /* destroy the existing sensor processes */
  if (s_pid > 0) {
    Destroy(s_pid);
  }
  if (c_pid > 0) {
    Destroy(c_pid);
  }
  /* then create new ones */
  s_pid = Create("sensors", MyPriority());
  c_pid = Create("sensors_courier", MyPriority());
}

