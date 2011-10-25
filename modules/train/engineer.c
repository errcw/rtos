#include <stdio.h>
#include <string.h>

#include <syscall.h>
#include <trainlib.h>
#include "heartbeat.h"
#include "engineer.h"
#include "track.h"

/* Math utilities. */
#define ABS(x) ((x) < 0 ? -(x) : (x))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

/* Convert speeds in mm/ms to cm/s. */
#define MMMS_TO_CMS(n) ((n) * 1000 / 10)
#define MM_TO_CM(n) ((n) / 10.0)

/* Milliseconds between updates. */
#define UPDATE_DELAY (25)

/* The speed we run our initialization routine. */
#define INITIALIZATION_SPEED (4)
#define INITIALIZATION_TIME_START (2000)

/* Error margins for sensors. */
#define SENSOR_ERROR (50 * 2)
#define SPEED_ERROR (1.35)
#define WINDOW_MIN (100)
#define WINDOW_CALIBRATION_UPDATES (10)
#define WINDOW_CALIBRATION_ERROR (10)
#define WINDOW_ACCELERATION_ERROR (2)

/* The speed/acceleration model. */
#define CALIBRATED_SPEED (8)

#define SPEED_SLOPE (0.0607)
#define SPEED_BASE (0.01311)
#define SPEED_LINE(s) (s * SPEED_SLOPE + SPEED_BASE)

#define ACCELERATION_BASE (0.000145)
#define ACCELERATION_SLOPE (0.000000563)
#define ACCELERATION_LINE(s) (s * s * ACCELERATION_SLOPE + ACCELERATION_BASE)

/* Reverse offset. */
#define REVERSE_OFFSET(s) (210 + (s - 8) * 25)

/* Journey. */
#define JOURNEY_REVERSE_OFFSET (150)

/* The collision distance window. */
#define RESERVATION_WINDOW (150)

/* Distance window before we consider ourselves lost. */
#define LOST_WINDOW (600)

/* The train number to control. */
static int number;

/* Our current speed. */
static int speed_num;

/* Speed tracking. */
static float cur_speed;
static float last_cur_speed;
static float track_speed;
static float acceleration;
static int accelerating;
static int was_accelerating;

/* The average speed in mm/ms for each speed level. */
static float avg_speeds[TRAIN_SPEED_NUM];
static int avg_speed_updates[TRAIN_SPEED_NUM];

/* Recorded track times. */
static float track_speeds[TRAIN_SPEED_NUM][SENSORS_MAX][SENSORS_MAX];
static int track_speed_updates[TRAIN_SPEED_NUM][SENSORS_MAX][SENSORS_MAX];

/* How we interpret the default speeds. */
static float speed_multiplier;

/* The last sensor hit. */
static int last_sensor;
static Time_t last_sensor_time;

/* The next sensors we expect to hit. */
static int next_sensor;
static int next_sensor_dist;
static int next_sensor_fired;

static int nextnext_sensor;
static int nextnext_sensor_dist;
static int nextnext_sensor_fired;

static int nextnextnext_sensor;
static int nextnextnext_sensor_dist;
static int nextnextnext_sensor_fired;

/* The switch on our path. */
static int next_sw;
static int next_sw_dist;
static int next_sw_state;
static int last_sw;
static int last_sw_state;

/* Alternate path tracking. */
static int alternate_path;
static int alternate_path_start;
static int alternate_path_end;

/* The last point traversed. */
static struct {
  int type;
  int id;
} last_pt;

/* Track distance. */
static float distance;
static float sensor_distance;
static float sensor_distance_last;

/* The train position. */
static TrainPosition_t position;
static Time_t last_update;
static int position_lost;

/* If we are initializing. */
static int initializing;
static int init_time;
static int init_elapsed_time;

/* The track manager. */
static int track_pid;
/* The engineer coordinator. */
static int coord_pid;

/* Path following. */
static EngineerDestination_t journey_dest;
static TrackPath_t journey;
static int journey_follow;
static int journey_next;
static int journey_reverse;
static int journey_stop;
static int journey_last_update;

/* Reservation handling. */
static int reservation_waiting;
static int reservation_prev_speed_num;
static int reservation_prev_dist;

/* The last blocking position. */
static int blocking_last;

/* Set up and run the program. */
static void Init ();
static void Run ();

/* Handle train commands. */
static void TrainPositionInit ();
static void TrainForward (EngineerForward_t *fwd);
static void TrainReverse ();
static void TrainSetDestination(EngineerDestination_t *dest);

static void TrainHorn ();
static void TrainLights (int on);

/* Track switch update. */
static void SwitchUpdate (int sw, int state);

/* Called when a sensor is triggered. */
static void EvaluateSensorFired (int sensor, int coord);
static int IsSensorOnPath (int sensor, float *distance);
static float GetDistanceWindow ();
static void SensorHit (int sensor);

/* Updates the next switch/sensor to be encountered. */
static void UpdateNextSwitch ();
static void UpdateNextSensor ();
static int GetNextSensor (int prev, int *next_id, int *next_dist);

/* Updates the state of the train. */
static void UpdateTrain ();
static void UpdatePosition (int elapsed);
static void UpdatePositionLost (int elapsed);
static void UpdateInitialization (int elapsed);
static void UpdateJourney (int elapsed);
static void UpdateTrackBlocking (int elapsed);
static void UpdateReservations (int elapsed);

static void SetBlocking (int sensor, int disconnect);
static int ReserveTrack (int sensor, float distance, int *collision);

/* Returns the position of the train. */
static void GetPosition (int asker_pid);

static void UpdateDistance (int elapsed);
static void UpdateSpeed (int elapsed);

/* Train speed handling. */
static void SetSpeed (float speed);
static float GetSpeed (int snum, int lsensor, int nsensor);
static float GetAverageSpeed (int snum);
static float GetCalibratedSpeed (int snum, int lsensor, int nsensor);
static void CalibrateSpeed (float speed, int snum, int lsensor, int nsensor);
static float GetStoppingDistance ();
static float GetStoppingDistanceForSpeed (float speed);
static float GetReverseOffset ();

/* Journey handling. */
static void JourneyFindPath ();
static void JourneySetCurrentSwitches ();
static void JourneySetSwitches (int start, int end);
static void JourneySensorHit (int sensor);
static void JourneyReverse ();
static int JourneyGetSensor (int next);
static float JourneyGetDistanceRemaining ();
static int JourneyCanStop ();

/* Returns the elapsed milliseconds between two times. */
static int GetElapsedMs (Time_t now, Time_t then);


int main ()
{
  Init();
  Run();
  Exit();
}

static void Init ()
{
  /* receive our train number */
  int creator = Receive((char *)&number, sizeof(number));

  /* clear the position */
  last_sensor = -1;
  next_sensor = -1;
  last_sw = -1;
  next_sw = -1;
  distance = 0.0;
  sensor_distance = 0.0;
  sensor_distance_last = 0.0;
  last_pt.type = -1;
  last_pt.id = -1;
  journey_follow = 0;
  last_update = GetTime();
  accelerating = was_accelerating = 0;
  position_lost = 0;
  blocking_last = -1;
  journey_last_update = -1;

  /* clear the speeds */
  memset(track_speeds, 0.0, TRAIN_SPEED_NUM*SENSORS_MAX*SENSORS_MAX);
  memset(track_speed_updates, 0, TRAIN_SPEED_NUM*SENSORS_MAX*SENSORS_MAX);
  memset(avg_speeds, 0.0, TRAIN_SPEED_NUM);
  memset(avg_speed_updates, 0, TRAIN_SPEED_NUM);
  cur_speed = 0.0;
  last_cur_speed = 0.0;
  track_speed = 0.0;

  /* use the type to find the speeds */
  switch (number) {
    case 26:
      speed_multiplier = 0.9; break;
    case 22:
    case 23:
      speed_multiplier = 0.85; break;
    case 25:
    case 46:
      speed_multiplier = 0.75; break;
    case 24:
      speed_multiplier = 0.71; break;

    default:
      speed_multiplier = 0.85; break;
  }

  track_pid = WhoIs("track");
  coord_pid = WhoIs("coordinator");

  /* register ourselves with the coordinator */
  EngineerCommand_t reg_cmd;
  reg_cmd.type = TRAIN_REGISTER;
  reg_cmd.c.id.pid = MyPid();
  reg_cmd.c.id.train = number;
  Send(coord_pid, (char *)&reg_cmd, sizeof(reg_cmd), 0, 0);

  /* create our heartbeat */
  HeartbeatCfg_t hb_cfg = { MyPid(), UPDATE_DELAY };
  Send(Create("heartbeat", MyPriority() + 1), (char *)&hb_cfg, sizeof(hb_cfg), 0, 0);

  Reply(creator, 0, 0);
}

static void Run ()
{
  EngineerCommand_t cmd;

  for (;;) {
    int pid = Receive((char *)&cmd, sizeof(cmd));
    switch (cmd.type) {
      case HEARTBEAT:
        Reply(pid, 0, 0);
        UpdateTrain();
        break;

      case TRAIN_INITIALIZE: 
        Reply(pid, 0, 0);
        TrainPositionInit();
        break;
      case TRAIN_FORWARD:
        Reply(pid, 0, 0);
        TrainForward(&cmd.c.fwd);
        break;
      case TRAIN_REVERSE:
        Reply(pid, 0, 0);
        TrainReverse();
        break;
      case TRAIN_DESTINATION:
        Reply(pid, 0, 0);
        TrainSetDestination(&cmd.c.dest);
        break;

      case TRAIN_TRACK_SWITCH:
        Reply(pid, 0, 0);
        SwitchUpdate(cmd.c.sw.sw, cmd.c.sw.state);
        break;

      case TRAIN_TRACK_SENSOR:
        EvaluateSensorFired(cmd.c.se.sensor, pid);
        break;
      case TRAIN_GOTSENSOR:
        Reply(pid, 0, 0);
        SensorHit(cmd.c.se.sensor);
        break;

      case TRAIN_POSITION:
        GetPosition(pid);
        break;

      default:
        printf("engineer %d: unreconized command %d\n", number, cmd.type);
        break;
    }
  }
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
  sprintf(sname, "%c%d", module, snum);
}

static void TrainPositionInit ()
{
  journey_follow = 0;
  reservation_waiting = 0;

  last_sensor = next_sensor = -1;
  distance = sensor_distance = 0;

  initializing = 1;
  init_time = INITIALIZATION_TIME_START;
  init_elapsed_time = 0;

  TrainHorn();

  EngineerForward_t fwd = { INITIALIZATION_SPEED };
  TrainForward(&fwd);
}

static void TrainForward (EngineerForward_t *fwd)
{
  if (reservation_waiting) {
    /* bail if we are held on a reservation */
    return;
  }

  if (speed_num != fwd->speed) {
    TrainCommand_t cmd;
    cmd.type = TRAIN_CMD_FORWARD;
    cmd.id = number;
    cmd.param = fwd->speed;
    TrainExecCmd(&cmd);

    /* update the internal speed measure */
    speed_num = fwd->speed;
    accelerating = 1;
    SetSpeed( GetSpeed(speed_num, last_sensor, next_sensor) );
  }
}

static void TrainReverse ()
{
  TrainCommand_t cmd;
  cmd.type = TRAIN_CMD_REVERSE;
  cmd.id = number;
  cmd.param = speed_num;
  TrainExecCmd(&cmd);

  /* find the new naming point */
  if (last_pt.id >= 0) {
    TrackQueryReply_t qr;
    TrackCommand_t tr_cmd;
    tr_cmd.c.q.type = last_pt.type;
    tr_cmd.c.q.id = last_pt.id;
    tr_cmd.type = TRACK_GETNEXT_INVERTED;

    Send(track_pid, (char *)&tr_cmd, sizeof(tr_cmd), (char *)&qr, sizeof(qr));

    last_pt.type = qr.type;
    last_pt.id = qr.position;
    next_sw_state = qr.sw_state;

    distance = qr.distance - distance;
  }

  /* invert the next sensor */
  int s = next_sensor;
  int l = last_sensor;
  next_sensor = (l >= 0) ? TRACK_OPPOSITE_SENSOR(l) : l;
  last_sensor = (s >= 0) ? TRACK_OPPOSITE_SENSOR(s) : s;
  sensor_distance = (s >= 0) ? next_sensor_dist - sensor_distance : distance;
  /* and find the nextnext, nextnextnext */
  UpdateNextSensor();
  UpdateNextSwitch();

  /* adjust the position */
  if (speed_num != 0) {
    sensor_distance -= GetReverseOffset();
    cur_speed = 0;
    accelerating = 1;
  }

  /* reset our reservations */
  reservation_waiting = 0;
}

static void TrainSetDestination (EngineerDestination_t *dest)
{
  journey_dest = *dest;

  EngineerForward_t fwd = { dest->speed };
  TrainForward(&fwd);

  JourneyFindPath();

  TrainLights(1);
}

static void TrainHorn ()
{
  TrainCommand_t cmd;
  cmd.type = TRAIN_CMD_FUNCTION;
  cmd.id = number;
  cmd.param = TRAIN_FUNCTION_HORN;
  TrainExecCmd(&cmd);
}

static void TrainLights (int on)
{
  TrainCommand_t cmd;
  cmd.type = TRAIN_CMD_FUNCTION;
  cmd.id = number;
  cmd.param = on ? TRAIN_FUNCTION_LIGHTS : TRAIN_FUNCTION_OFF;
  TrainExecCmd(&cmd);
}

static void UpdateTrain ()
{
  Time_t now = GetTime();
  int elapsed = GetElapsedMs(now, last_update);
  if (elapsed <= 0) {
    return;
  }

  UpdatePosition(elapsed);
  UpdatePositionLost(elapsed);
  UpdateInitialization(elapsed);
  UpdateJourney(elapsed);
  UpdateTrackBlocking(elapsed);
  UpdateReservations(elapsed);

  last_update = now;
}

static void UpdatePosition (int elapsed)
{
  /* find the train's position and speed */
  UpdateDistance(elapsed);
  UpdateSpeed(elapsed);

  /* check if we have passed a switch */
  if (next_sw >= 0 && next_sw != last_sw && distance >= next_sw_dist) {
    last_sw = next_sw;
    last_sw_state = next_sw_state;
    last_pt.type = NODE_SWITCH;
    last_pt.id = last_sw;

    UpdateNextSwitch();

    distance = 0.0;
  }

  /* generate the position information */
  position.position = last_pt.id;
  position.position_type = last_pt.type;
  position.switch_state = last_sw_state;
  position.distance = (int)MM_TO_CM(distance);

  /* generate the next sensor information */
  position.sensor_valid = (next_sensor >= 0 && speed_num > 0);
  if (position.sensor_valid) {
    position.sensor = next_sensor;
    position.sensor_dist = next_sensor_dist;

    float distance_left = next_sensor_dist - sensor_distance;
    if (distance_left < 0) {
      position.sensor_time.ms = 0;
      position.sensor_time.sec = 0;
      position.sensor_time.min = 0;
    } else {
      int time_left = (int)(distance_left / cur_speed);
      position.sensor_time.ms = time_left % 1000;
      position.sensor_time.sec = (time_left / 1000) % 60;
      position.sensor_time.min = time_left / (1000 * 60);
    }
  }

  /* generate the speed information */
  position.speed = (int)MMMS_TO_CMS(cur_speed);

  /* generate the journey information */
  position.journey_next = JourneyGetSensor(0);
  position.journey_valid = journey_follow;
}

static void UpdatePositionLost (int elapsed)
{
  if (initializing || position_lost) {
    /* bail if we are initializing or already lost */
    return;
  }

  /* check if we have overrun our distance expectations */
  if (sensor_distance > next_sensor_dist + nextnext_sensor_dist + nextnextnext_sensor_dist + LOST_WINDOW) {
    printf("engineer %d (%d): lost, requesting new position\n", MyPid(), number);
    EngineerCommand_t cmd;
    cmd.type = TRAIN_POSITION_LOST;
    cmd.c.id.pid = MyPid();
    cmd.c.id.train = number;
    Send(coord_pid, (char *)&cmd, sizeof(cmd), 0, 0);

    position_lost = 1;
  }
}

static void UpdateInitialization (int elapsed)
{
  if (!initializing) {
    return;
  }

  /* reverse on an exponential time scale */
  init_elapsed_time += elapsed;
  if (init_elapsed_time >= init_time) {
    init_time *= 2;
    init_elapsed_time = 0;
    TrainReverse();
  }
}

static void UpdateJourney (int elapsed)
{
  if (!journey_follow) {
    return;
  }

  /* update after every sensor */
  if (last_sensor != journey_last_update) {
    JourneyFindPath();
    journey_last_update = last_sensor;
  }

  /* reverse when far enough past a sensor */
  if (journey_reverse && (sensor_distance + GetReverseOffset()) >= JOURNEY_REVERSE_OFFSET) {
    JourneyReverse();
    return;
  }

  /* stop when distance is enough */
  if (JourneyCanStop() && JourneyGetDistanceRemaining() <= GetStoppingDistance()) {
    journey_stop = 1;
    EngineerForward_t fwd = { 0 };
    TrainForward(&fwd);
  }

  /* end the journey when we have stopped */
  if (journey_stop && cur_speed == 0) {
    journey_follow = 0;
    TrainLights(0);
  }
}

static void UpdateTrackBlocking (int elapsed)
{
  if (last_sensor != blocking_last) {
    /* restore the track */
    SetBlocking(blocking_last, 0);
    /* set a new block */
    SetBlocking(last_sensor, 1);
    blocking_last = last_sensor;
  }
}

static void SetBlocking (int sensor, int disconnect)
{
  if (sensor < 0) {
    return;
  }
  TrackCommand_t cmd;
  cmd.type = TRACK_MODPATH;
  cmd.c.pm.sensor = sensor;
  cmd.c.pm.disconnect = disconnect;
  Send(track_pid, (char *)&cmd, sizeof(cmd), 0, 0);
}

static void UpdateReservations (int elapsed)
{
  /* do not reserve during initialization */
  if (initializing) {
    return;
  }

  /* update the track reservation */
  float stop_dist = GetStoppingDistanceForSpeed(track_speed);
  float reserve_dist = (reservation_waiting)
    ? reservation_prev_dist
    : stop_dist + sensor_distance + RESERVATION_WINDOW;

  int reservation_col;
  int reservation_ok = ReserveTrack(last_sensor, reserve_dist, &reservation_col);

  /* act on the reservations */
  if (reservation_ok && reservation_waiting) {
    if (cur_speed == 0) {
      reservation_waiting = 0;

      EngineerForward_t fwd = { reservation_prev_speed_num };
      TrainForward(&fwd);
    }

  } else if (!reservation_ok) {
    if (!reservation_waiting) {
      printf("engineer %d: collision\n", number);
      if (speed_num > 0) {
        reservation_prev_speed_num = speed_num;
        reservation_prev_dist = reserve_dist;
        EngineerForward_t fwd = { 0 };
        TrainForward(&fwd);

        reservation_waiting = 1;
      }
    }

  }
}

static int ReserveTrack (int sensor, float distance, int *collision)
{
  EngineerCommand_t cmd;
  cmd.type = TRAIN_RESERVE;
  cmd.c.res.start = sensor;
  cmd.c.res.distance = distance;

  EngineerReservationResponse_t resp;

  Send(coord_pid, (char *)&cmd, sizeof(cmd), (char *)&resp, sizeof(resp));

  if (collision) {
    *collision = resp.collision;
  }

  return (resp.type == RESERVATION_OK);
}

static void GetPosition (int asker_pid)
{
  Reply(asker_pid, (char *)&position, sizeof(position));
}

static void SwitchUpdate (int sw, int state)
{
  if (sw == next_sw) {
    next_sw_state = state;
  }
  /* always update our sensors */
  UpdateNextSensor();
}

static void EvaluateSensorFired (int sensor, int coord)
{
  EngineerSensorEval_t eval;

  char sname[10]; GetSensorName(sensor, sname);
  printf("engineer %d: eval %s ", number, sname);

  float window = GetDistanceWindow();
  float branch_distance;

  if (initializing) {
    /* accept any sensor during initialization */
    eval.type = SENSOR_EXPECTED;
    eval.distance = 0;
    eval.inside_window = 1;
    printf("(i)");

  } else if (cur_speed == 0.0) {
    /* never expect to hit sensors while stationary */
    eval.type = SENSOR_UNEXPECTED;
    printf("(z)");

  } else if (sensor == next_sensor) {
    /* expected the next sensor */
    eval.type = SENSOR_EXPECTED;
    eval.distance = ABS(sensor_distance - next_sensor_dist);
    eval.inside_window = 1;
    printf("(n) disterr=%f, ok=%d", eval.distance, eval.inside_window);

    next_sensor_fired = 1;
    sensor_distance_last = 0.0;

  } else if (sensor == nextnext_sensor) {
    /* evaluate the sensor after the next */
    if (next_sensor_fired) {
      eval.type = SENSOR_EXPECTED;
      eval.distance = ABS(sensor_distance_last - nextnext_sensor_dist);
    } else {
      eval.type = SENSOR_POTENTIAL;
      eval.distance = ABS(sensor_distance - next_sensor_dist - nextnext_sensor_dist);
    }
    eval.inside_window = (eval.distance <= window*2);
    printf("(nn) disterr=%f, ok=%d", eval.distance, eval.inside_window);

    nextnext_sensor_fired = 1;
    sensor_distance_last = 0.0;

  } else if (sensor == nextnextnext_sensor) {
    /* evaluate the sensor after the after the next */
    if (nextnext_sensor_fired) {
      eval.type = SENSOR_EXPECTED;
      eval.distance = ABS(sensor_distance_last - nextnextnext_sensor_dist);
    } else {
      eval.type = SENSOR_POTENTIAL;
      eval.distance = ABS(sensor_distance - next_sensor_dist - nextnext_sensor_dist - nextnextnext_sensor_dist);
    }
    eval.inside_window = (eval.distance <= window);
    printf("(nnn) disterr=%f, ok=%d", eval.distance, eval.inside_window);

    nextnextnext_sensor_fired = 1;
    sensor_distance_last = 0.0;

  } else if (IsSensorOnPath(sensor, &branch_distance)) {
    /* evaluate the sensor on a branched path */
    eval.type = SENSOR_POTENTIAL;
    eval.distance = ABS(sensor_distance - branch_distance);
    eval.inside_window = (eval.distance <= window);
    printf("(p) disterr=%f, ok=%d", eval.distance, eval.inside_window);

    alternate_path = 1;
    alternate_path_start = last_sensor;
    alternate_path_end = sensor;

  } else {
    eval.type = SENSOR_UNEXPECTED;
    printf("(u)");

  }
  printf("\n");

  Reply(coord, (char *)&eval, sizeof(eval));
}

static void SensorHit (int sensor)
{
  /* handle initialization */
  if (initializing) {
    initializing = 0;
    EngineerForward_t fwd = { 0 };
    TrainForward(&fwd);
  }
  
  /* handle branching paths */
  if (alternate_path && alternate_path_end == sensor) {
    TrackCommand_t cmd;
    cmd.type = TRACK_UPDATEPATH;
    cmd.c.pq.start = alternate_path_start;
    cmd.c.pq.end = alternate_path_end;
    cmd.c.pq.exec = 0;
    Send(track_pid, (char *)&cmd, sizeof(cmd), 0, 0);
    alternate_path = 0;
  }

  /* calibrate speeds for this section */
  if (sensor == next_sensor && !accelerating && !was_accelerating) {
    int time = GetElapsedMs(GetTime(), last_sensor_time);
    if (time > 0) {
      float speed = next_sensor_dist / (float)time;
      CalibrateSpeed(speed, speed_num, last_sensor, sensor);
    }
  } else {
    printf("engineer %d: omitted speed calibration\n", number);
  }
  if (!accelerating) {
    was_accelerating = 0;
  }

  /* remember the last hit */
  last_sensor = sensor;
  last_sensor_time = GetTime();
  last_pt.type = NODE_SENSOR;
  last_pt.id = last_sensor;

  /* find the next positions */
  UpdateNextSwitch();
  UpdateNextSensor();

  /* update the speed for this section */
  SetSpeed(GetSpeed(speed_num, sensor, next_sensor));

  /* reset the distance measures */
  distance = 0.0;
  sensor_distance = 0.0;

  /* we are no longer lost */
  position_lost = 0;

  /* handle the journey */
  if (journey_follow) {
    JourneySensorHit(sensor);
  }
}

static int IsSensorOnPath (int sensor, float *distance)
{
  if (last_sensor >= 0) {
    TrackCommand_t cmd;
    TrackQueryReply_t qr;

    cmd.type = TRACK_ISONPATH;
    cmd.c.pq.start = last_sensor;
    cmd.c.pq.end = sensor;

    Send(track_pid, (char *)&cmd, sizeof(cmd), (char *)&qr, sizeof(qr));

    if (qr.type == NODE_SENSOR) {
      *distance = qr.distance;
      return 1;
    }
  }
  return 0;
}

static float GetDistanceWindow ()
{
  /* calculate the default window for sensor delay */
  float window = track_speed * SENSOR_ERROR * SPEED_ERROR;

  /* adjust for acceleration */
  if (accelerating || was_accelerating) {
    window *= WINDOW_ACCELERATION_ERROR;
  }

  /* adjust for calibration */
  int updates = avg_speed_updates[speed_num];
  float calib_comp = MAX(WINDOW_CALIBRATION_UPDATES - updates, 0) * WINDOW_CALIBRATION_ERROR;
  window += calib_comp;

  /* ensure a minimum window */
  window = MAX(window, WINDOW_MIN);

  return window;
}

static void UpdateNextSwitch ()
{
  if (last_pt.id < 0) {
    return;
  }

  TrackQueryReply_t qr;

  TrackCommand_t cmd;
  cmd.c.q.type = last_pt.type;
  cmd.c.q.id = last_pt.id;
  cmd.type = TRACK_GETNEXT;

  Send(track_pid, (char *)&cmd, sizeof(cmd), (char *)&qr, sizeof(qr));

  if (qr.type == NODE_SWITCH) {
    next_sw = qr.position;
    next_sw_dist = qr.distance;
    next_sw_state = qr.sw_state;
  } else {
    next_sw = -1;
  }
}

static void UpdateNextSensor ()
{
  if (last_sensor >= 0) {
    GetNextSensor(last_sensor, &next_sensor, &next_sensor_dist);
    next_sensor_fired = 0;
  }
  if (next_sensor >= 0) {
    if (GetNextSensor(next_sensor, &nextnext_sensor, &nextnext_sensor_dist)) {
      nextnext_sensor_fired = 0;
      if (GetNextSensor(nextnext_sensor, &nextnextnext_sensor, &nextnextnext_sensor_dist)) {
        nextnextnext_sensor_fired = 0;
      }
    }
  }
}

static int GetNextSensor (int prev, int *next_id, int *next_dist)
{
  if (prev < 0) {
    return 0;
  }

  TrackQueryReply_t qr;

  TrackCommand_t cmd;
  cmd.c.q.type = NODE_SENSOR;
  cmd.c.q.id = prev;
  cmd.type = TRACK_GETNEXT_SENSOR;

  Send(track_pid, (char *)&cmd, sizeof(cmd), (char *)&qr, sizeof(qr));

  if (qr.type == NODE_SENSOR) {
    *next_id = qr.position;
    *next_dist = qr.distance;
    return 1;
  } else {
    *next_id = -1;
    *next_dist = 0;
    return 0;
  }
}

static void UpdateDistance (int elapsed)
{
  float dd = cur_speed * elapsed;
  if (cur_speed != track_speed) {
    dd += 0.5 * acceleration * elapsed * elapsed;
  }
  distance += dd;
  sensor_distance += dd;
  sensor_distance_last += dd;
}

static void UpdateSpeed (int elapsed)
{
  last_cur_speed = cur_speed;

  if (cur_speed == track_speed) {
    return;
  }

  float a = (track_speed > cur_speed) ? acceleration : -acceleration;

  cur_speed += a * elapsed;
  if ((a > 0 && cur_speed > track_speed) ||
      (a < 0 && cur_speed < track_speed)) {
    cur_speed = track_speed;
  }

  if (accelerating && cur_speed == track_speed) {
    accelerating = 0;
    was_accelerating = 1;
  }
}

static void CalibrateSpeed (float speed, int snum, int lsensor, int nsensor)
{
  /* filter out bogus speeds */
  if (speed == 0.0) {
    printf("engineer %d: attempted to calibrate speed of zero\n", number);
    return;
  }

  /* update the speed for the given section of track */
  if (track_speed_updates[snum][lsensor][nsensor] > 0) {
    float prev_speed = track_speeds[snum][lsensor][nsensor];
    float rel_error = ABS(speed - prev_speed) / speed;
    if (rel_error < 0.1) {
      track_speeds[snum][lsensor][nsensor] = prev_speed*0.25 + speed*0.75;
    } else if (rel_error < 0.5) {
      track_speeds[snum][lsensor][nsensor] = prev_speed*0.75 + speed*0.25;
    } else {
      printf("engineer %d: rejected speed %f (err=%f)\n", number, speed, rel_error);
    }
  } else {
    float average = GetAverageSpeed(snum);
    track_speeds[snum][lsensor][nsensor] = average*0.75 + speed*0.25;
  }
  track_speed_updates[snum][lsensor][nsensor] += 1;

  /* update the average speed */
  float average = GetAverageSpeed(snum);
  float avg_error = ABS(speed - average) / speed;
  if (avg_error < 0.15) {
    avg_speeds[snum] = average*0.5 + speed*0.5;
  } else {
    avg_speeds[snum] = average*0.75 + speed*0.25;
  }
  avg_speed_updates[snum] += 1;
}

static void SetSpeed (float speed)
{
  track_speed = speed;
  if (!accelerating) {
    /* directly set the current speed when not accelerating */
    cur_speed = speed;
  }
  if (track_speed > cur_speed) {
    /* set acceleration only when accelerating */
    acceleration = ACCELERATION_LINE(speed_num) * speed_multiplier;
  }
}

static float GetSpeed (int snum, int lsensor, int nsensor)
{
  /* zero always maps to zero */
  if (snum == 0) {
    return 0.0;
  }
  /* try to get a calibrated speed */
  float speed = GetCalibratedSpeed(snum, lsensor, nsensor);
  if (speed == 0.0) {
    /* otherwise fall back to the average */
    speed = GetAverageSpeed(snum);
  }
  return speed;
}

static float GetCalibratedSpeed (int snum, int lsensor, int nsensor)
{
  if (lsensor >= 0 && nsensor >= 0) {
    if (track_speed_updates[speed_num][lsensor][nsensor]) {
      /* get the calibrated speed for the given track section */
      return track_speeds[speed_num][lsensor][nsensor];

    } else if (track_speed_updates[speed_num][nsensor][lsensor]) {
      /* try the opposite direction */
      return track_speeds[speed_num][nsensor][lsensor];

    } else if (track_speed_updates[CALIBRATED_SPEED][lsensor][nsensor]) {
      /* try the scaled default speed */
      return track_speeds[CALIBRATED_SPEED][lsensor][nsensor] + (speed_num - CALIBRATED_SPEED) * SPEED_SLOPE*speed_multiplier;

    }
  }

  return 0.0;
}

static float GetAverageSpeed (int snum)
{
  if (avg_speed_updates[snum] > 0) {
    /* get the average for the given speed */
    return avg_speeds[snum];

  } else {
    /* otherwise use the equation */
    return SPEED_LINE(snum) * speed_multiplier;

  }
}

static float GetStoppingDistance ()
{
  return GetStoppingDistanceForSpeed(cur_speed);
}

static float GetStoppingDistanceForSpeed (float speed)
{
  float time = (speed / acceleration);
  float dist = speed * time + 0.5 * -acceleration * time * time;
  return dist;
}

static float GetReverseOffset ()
{
  switch (speed_num) {
    case 14: return 520;
    case 13: return 500;
    case 12: return 410;
    case 11: return 300;
    case 10: return 370;
    case 9: return 270;
    case 8: return 210;
    case 7: return 200;
    default: return 200 * (speed_num / 7.0);
  }
}

static void JourneyFindPath ()
{
  TrackCommand_t cmd;
  cmd.type = TRACK_GETPATH;

  cmd.c.path.start_fwd = next_sensor;
  cmd.c.path.start_fwd_dist = next_sensor_dist - sensor_distance;
  cmd.c.path.start_rev = (last_sensor >= 0) ? TRACK_OPPOSITE_SENSOR(last_sensor) : -1;
  cmd.c.path.start_rev_dist = (last_sensor >= 0) ? sensor_distance : -1;

  cmd.c.path.end = journey_dest.id;
  cmd.c.path.end_type = journey_dest.type;
  cmd.c.path.end_offset = journey_dest.offset;
  cmd.c.path.end_sw_state = journey_dest.sw_state;
  
  SetBlocking(blocking_last, 0);

  Send(track_pid, (char *)&cmd, sizeof(cmd), (char *)&journey, sizeof(journey));

  SetBlocking(blocking_last, 1);

  if (journey.size == 0) {
    printf("engineer %d: got empty path\n", number);
    return;
  } else {
    char sname[10];
    printf("engineer %d: got path ", number);
    for (int i = journey.size - 1; i >= 0; i--) {
      GetSensorName(journey.path[i].sensor, sname);
      printf("%s(%d) ", sname, journey.path[i].dist_remaining);
    }
    printf("\n");
  }

  /* reset the state */
  journey_follow = 1;
  journey_reverse = 0;
  journey_stop = 0;
  journey_next = journey.size - 1;

  if (journey.path[0].sensor == last_sensor) {
    journey_next = -1;
  }

  /* switch the initial switches */
  JourneySetCurrentSwitches();

  /* reverse if necessary */
  if (JourneyGetSensor(0) == TRACK_OPPOSITE_SENSOR(last_sensor)) {
    TrainReverse();
  }
}

static void JourneySetCurrentSwitches ()
{
  JourneySetSwitches(JourneyGetSensor(0), JourneyGetSensor(1));
  JourneySetSwitches(JourneyGetSensor(1), JourneyGetSensor(0));
}

static void JourneySetSwitches (int start, int end)
{
  if (start < 0 || end < 0) {
    return;
  }
  TrackCommand_t cmd;
  cmd.type = TRACK_UPDATEPATH;
  cmd.c.pq.start = start;
  cmd.c.pq.end = end;
  cmd.c.pq.exec = 1;
  Send(track_pid, (char *)&cmd, sizeof(cmd), 0, 0);
}

static void JourneySensorHit (int sensor)
{
  /* check if we need to reverse */
  if (journey_reverse) {
    JourneyReverse();
    return;
  }

  /* find the sensor on the journey */
  int idx = -1;
  for (int i = 0; i < journey.size; i++) {
    if (journey.path[i].sensor == sensor) {
      idx = i;
      break;
    }
  }

  if (idx > 0) {
    /* note the position in the journey */
    journey_next = idx - 1;

    int next = JourneyGetSensor(0);
    if (next == TRACK_OPPOSITE_SENSOR(sensor)) {
      /* reverse and wait to switch */
      journey_reverse = 1;
    } else {
      /* switch ahead of ourselves */
      JourneySetCurrentSwitches();
    }

  } else if (idx == 0) {
    /* reached last sensor in journey */
    journey_next = -1;

  } else {
    /* lost our way on the journey */
    JourneyFindPath();
  }
}

static void JourneyReverse ()
{
  journey_reverse = 0;
  JourneySetCurrentSwitches();
  TrainReverse();
}

static int JourneyGetSensor (int next)
{
  if (journey_next - next >= 0) {
    return journey.path[journey_next - next].sensor;
  } else {
    return -1;
  }
}

static float JourneyGetDistanceRemaining ()
{
  if (journey_next >= 0) {
    float dist_to_sensor = MAX(next_sensor_dist - sensor_distance, 0);
    return journey.path[journey_next].dist_remaining + dist_to_sensor;
  } else {
    /* passed last sensor, only use remaining distance */
    return MAX(journey.path[0].dist_remaining - sensor_distance, 0);
  }
}

static int JourneyCanStop ()
{
  /* cannot stop twice */
  if (journey_stop) {
    return 0;
  }
  /* cannot stop if there is a reversal yet to occur */
  for (int i = 0; i < journey_next; i++) {
    if (JourneyGetSensor(i) == TRACK_OPPOSITE_SENSOR(JourneyGetSensor(i + 1))) {
      return 0;
    }
  }
  return 1;
}

static int GetElapsedMs (Time_t now, Time_t then)
{
  int then_ms = then.ms + then.sec*1000 + then.min*1000*60;
  int now_ms = now.ms + now.sec*1000 + now.min*1000*60;
  return now_ms - then_ms;
}

