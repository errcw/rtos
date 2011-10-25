#include <stdio.h>
#include <string.h>

#include <trainlib.h>
#include <syscall.h>
#include <terminal.h>
#include "track.h"
#include "tracka.h"
#include "trackb.h"

#define MAX_DISTANCE (1000000)

/* Initial switch states. */
static unsigned char switch_init[] =
{
  1, SWITCH_STRAIGHT,
  2, SWITCH_STRAIGHT,
  3, SWITCH_CURVED,
  4, SWITCH_STRAIGHT,
  5, SWITCH_CURVED,
  6, SWITCH_STRAIGHT,
  7, SWITCH_STRAIGHT,
  8, SWITCH_STRAIGHT,
  9, SWITCH_STRAIGHT,
  10, SWITCH_STRAIGHT,
  11, SWITCH_CURVED,
  12, SWITCH_CURVED,
  13, SWITCH_STRAIGHT,
  14, SWITCH_STRAIGHT,
  15, SWITCH_STRAIGHT,
  16, SWITCH_STRAIGHT,
  17, SWITCH_STRAIGHT,
  18, SWITCH_CURVED,
  153, SWITCH_CURVED,
  154, SWITCH_CURVED,
  155, SWITCH_CURVED,
  156, SWITCH_CURVED,
};

/* The current track model. */
static TrackModel_t *track;

/* Pathfinding structures. */
static int path_dist_model[SENSORS_MAX][SENSORS_MAX];
static int path_pred_model[SENSORS_MAX][SENSORS_MAX];
static int path_dist[SENSORS_MAX][SENSORS_MAX];
static int path_pred[SENSORS_MAX][SENSORS_MAX];
static int path_disconnections[SENSORS_MAX][SENSORS_MAX];

/* The outgoing event queue. */
#define EVENTQ_MAX (100)
static struct {
  TrackCommand_t cmds[EVENTQ_MAX];
  unsigned int head;
  unsigned int tail;
  unsigned int size;
} eventq;

/* Initialize and run the server. */
static void Init ();
static void Run ();

/* Initializes the switches to a known state. */
static void TrackInit (TrackCfg_t *cfg);
/* Receives a sensor state notification. */
static void TrackSensor (TrackSensorState_t *sensors);
/* Sets a switch state. */
static void TrackSwitch (TrackSwitchState_t *sw);
/* Adds an observer. */
static void TrackAddObserver (TrackObserver_t *obs);

/* Returns the next data. */
static void TrackGetNext (int querier, TrackQuery_t *q);
static void TrackGetNextInverted (int querier, TrackQuery_t *q);
static void TrackGetNextSensor (int querier, TrackQuery_t *q);
static void TrackGetNextSensors (int querier, TrackSensorQuery_t *q);
/* Handles alternate paths. */
static void TrackIsOnPath (int querier, TrackPathQuery_t *q);
static void TrackUpdatePath (int querier, TrackPathQuery_t *q);

static void GetNext (TrackQuery_t *q, TrackQueryReply_t *reply);
static void GetNextInverted (TrackQuery_t *q, TrackQueryReply_t *reply);
static void GetNextSensor (TrackQuery_t *q, TrackQueryReply_t *reply);
static int IsOnPath (int sensor, int prev_idx, TrackEdge_t *e);
static int UpdatePath (int sensor, int prev_idx, TrackEdge_t *e, int exec);
static int GetEdge (TrackQuery_t *q, TrackEdge_t **e);

/* Pathfinding. */
static void InitPathfinding ();
static void UpdatePathfinding (TrackPathModification_t *pm);
static void TrackFindPath (int pid, TrackPathfindRequest_t *pr);
static void GetPathStart (TrackPathfindRequest_t *pr, int end, int *start);
static void GetPathEnd (TrackPathfindRequest_t *pr, int *end, int *offset);

/* Adds an event to the event queue. */
static void QueueEvent (TrackCommand_t *cmd);
/* Sends events to our observers via the courier. */
static void BroadcastEvents (int courier_pid);
/* The waiting courier, if any. */
static int courier_pid;


int main ()
{
  Init();
  Run();
  Exit();
}

static void Init ()
{
  RegisterAs("track");

  eventq.head = 0;
  eventq.tail = 0;
  eventq.size = 0;
  courier_pid = 0;
}

static void Run ()
{
  TrackCommand_t command;
  for (;;) {
    int pid = Receive((char *)&command, sizeof(command));

    switch (command.type) {
      /* track state updates */
      case TRACK_CONFIG:
        TrackInit(&command.c.cfg);
        InitPathfinding();
        Reply(pid, 0, 0);
        break;
      case TRACK_SENSOR:
        TrackSensor(&command.c.sens);
        Reply(pid, 0, 0);
        break;
      case TRACK_SWITCH:
        TrackSwitch(&command.c.sw);
        Reply(pid, 0, 0);
        break;

      /* track state queries */
      case TRACK_GETNEXT:
        TrackGetNext(pid, &command.c.q);
        break;
      case TRACK_GETNEXT_INVERTED:
        TrackGetNextInverted(pid, &command.c.q);
        break;
      case TRACK_GETNEXT_SENSOR:
        TrackGetNextSensor(pid, &command.c.q);
        break;
      case TRACK_GETNEXT_SENSORS:
        TrackGetNextSensors(pid, &command.c.sq);
        break;
      case TRACK_ISONPATH:
        TrackIsOnPath(pid, &command.c.pq);
        break;
      case TRACK_UPDATEPATH:
        TrackUpdatePath(pid, &command.c.pq);
        break;
      case TRACK_GETPATH:
        TrackFindPath(pid, &command.c.path);
        break;
      case TRACK_MODPATH:
        Reply(pid, 0, 0);
        UpdatePathfinding(&command.c.pm);
        break;

      /* track events */
      case TRACK_OBSERVER:
        Reply(pid, 0, 0);
        TrackAddObserver(&command.c.obs);
        break;
      case TRACK_COURIER:
        BroadcastEvents(pid);
        break;

      default:
        printf("track: unknown command %d\n", command.type);
        break;
    }
  }
}

static void TrackInit (TrackCfg_t *cfg)
{
  /* select the track model */
  track = (TrackModel_t *)((cfg->type == 'A') ? &tracka_data : &trackb_data);

  /* set the physical switch state */
  TrainCommand_t cmd;
  cmd.type = TRAIN_CMD_SWITCH;
  for (int i = 0; i < sizeof(switch_init); i += 2) {
    cmd.id = switch_init[i];
    cmd.param = switch_init[i+1];
    TrainExecCmd(&cmd);
  }
  /* final switch command */
  cmd.type = TRAIN_CMD_SWITCH_END;
  TrainExecCmd(&cmd);

  /* update the model */
  TrackSwitchState_t swcfg;
  swcfg.exec = 0;
  for (int i = 0; i < sizeof(switch_init); i += 2) {
    swcfg.swnum = switch_init[i];
    swcfg.direction = switch_init[i+1];
    TrackSwitch(&swcfg);
  }
}

static void TrackSensor (TrackSensorState_t *sensors)
{
  static TrackSensorState_t old;

  TrackSensorState_t diff;
  int has_event = 0;

  /* check for a change */
  for (int mod = 0; mod < SENSOR_MODS_MAX; mod++) {
    diff.modules[mod].lo = (sensors->modules[mod].lo ^ old.modules[mod].lo) & sensors->modules[mod].lo;
    diff.modules[mod].hi = (sensors->modules[mod].hi ^ old.modules[mod].hi) & sensors->modules[mod].hi;
    if (diff.modules[mod].lo || diff.modules[mod].hi) {
      has_event = 1;
    }
  }

  if (has_event) { 
    /* broadcast an event */
    TrackCommand_t cmd;
    cmd.type = TRACK_SENSOR;
    cmd.c.sens = diff;
    QueueEvent(&cmd);
  }

  /* remember the new configuration */
  old = *sensors;
}

static void TrackSwitch (TrackSwitchState_t *sw)
{
  /* update the model */
  int sw_index = track->switch_nodes[sw->swnum];
  TrackSwitch_t *sw_model = &track->nodes[sw_index].d.sw;
  if (sw_model->direction != sw->direction) {
    sw_model->direction = sw->direction;

    /* broadcast an event */
    TrackCommand_t cmd;
    cmd.type = TRACK_SWITCH;
    cmd.c.sw = *sw;
    QueueEvent(&cmd);
  }

  /* run the command */
  if (sw->exec) {
    TrainCommand_t cmd;
    cmd.type = TRAIN_CMD_SWITCH;
    cmd.id = sw->swnum;
    cmd.param = sw->direction;
    TrainExecCmd(&cmd);
    cmd.type = TRAIN_CMD_SWITCH_END;
    TrainExecCmd(&cmd);
  }
}

static void TrackGetNext (int querier, TrackQuery_t *q)
{
  TrackQueryReply_t reply = { -1, -1, 0 };

  GetNext(q, &reply);

  Reply(querier, (char *)&reply, sizeof(reply));
}

static void TrackGetNextInverted (int querier, TrackQuery_t *q)
{
  TrackQueryReply_t reply = { -1, -1, 0 };

  GetNextInverted(q, &reply);

  Reply(querier, (char *)&reply, sizeof(reply));
}

static void TrackGetNextSensor (int querier, TrackQuery_t *q)
{
  TrackQueryReply_t reply = { -1, -1, 0 };

  GetNextSensor(q, &reply);

  Reply(querier, (char *)&reply, sizeof(reply));
}

static void TrackGetNextSensors (int querier, TrackSensorQuery_t *q)
{
  TrackSensorQueryReply_t reply;
  reply.size = 0;

  TrackQuery_t tq = { NODE_SENSOR, q->start };
  TrackQueryReply_t tqr = { 0, };

  for (;;) {
    GetNextSensor(&tq, &tqr);

    if (tqr.type == NODE_SENSOR) {
      reply.sensors[reply.size] = tqr.position;
      reply.size += 1;

      /* break when we have the requested distance */
      if (tqr.distance >= q->distance) {
        break;
      }

      /* branch outwards */
      TrackQuery_t tqb = { NODE_SENSOR, tqr.position };
      TrackEdge_t *e;
      int start_idx = GetEdge(&tqb, &e);
      for (int s = 0; s < SENSORS_MAX; s++) {
        int dist = IsOnPath(s, start_idx, e);
        if (dist && tqr.distance + dist <= q->distance) {
          reply.sensors[reply.size] = s;
          reply.size += 1;
        }
      }

      /* continue forwards */
      tq.type = NODE_SENSOR;
      tq.id = tqr.position;

    } else {
      /* end of connected sensors */
      break;

    }
  }

  Reply(querier, (char *)&reply, sizeof(reply));
}

static void TrackIsOnPath (int querier, TrackPathQuery_t *q)
{
  TrackQueryReply_t reply = { -1, -1, 0 };

  TrackEdge_t *start_edge;
  TrackQuery_t tq = { NODE_SENSOR, q->start };
  int start_idx = GetEdge(&tq, &start_edge);

  reply.distance = IsOnPath(q->end, start_idx, start_edge);
  if (reply.distance) {
    reply.type = NODE_SENSOR;
  }

  Reply(querier, (char *)&reply, sizeof(reply));
}

static void TrackUpdatePath (int querier, TrackPathQuery_t *q)
{
  TrackEdge_t *start_edge;
  TrackQuery_t tq = { NODE_SENSOR, q->start };
  int start_idx = GetEdge(&tq, &start_edge);
  UpdatePath(q->end, start_idx, start_edge, q->exec);

  Reply(querier, 0, 0);
}

static void GetNext (TrackQuery_t *q, TrackQueryReply_t *reply)
{
  TrackEdge_t *next;
  int prev_idx = GetEdge(q, &next);
  int next_dest = next->dest;
  reply->distance += next->distance;

  for (;;) {
    TrackNode_t *node = &track->nodes[next_dest];
    if (node->type == NODE_SENSOR) {
      /* find the next sensor to fire */
      TrackSensor_t *se = &node->d.se;
      if (se->edges[0].dest == prev_idx) {
        reply->position = node->id * 2 + 1;
      } else {
        reply->position = node->id * 2;
      }
      reply->type = NODE_SENSOR;
      break;

    } else if (node->type == NODE_STOP) {
      /* dead end */
      reply->type = NODE_STOP;
      reply->position = node->id;
      break;

    } else if (node->type == NODE_SWITCH) {
      TrackSwitch_t *sw = &node->d.sw;
      if (sw->behind.dest == prev_idx) {
        /* arrived at a split */
        reply->type = NODE_SWITCH;
        reply->position = node->id;
        reply->sw_state = sw->direction;
        break;

      } else {
        /* traverse the switch */
        prev_idx = next_dest;
        next_dest = sw->behind.dest;
        reply->distance += sw->behind.distance;
      }

    }
  }
}

static void GetNextInverted (TrackQuery_t *q, TrackQueryReply_t *reply)
{
  TrackEdge_t *next;
  int prev_idx = GetEdge(q, &next);
  int next_dest = next->dest;
  reply->distance += next->distance;

  for (;;) {
    TrackNode_t *node = &track->nodes[next_dest];
    if (node->type == NODE_SENSOR) {
      /* find the opposite next sensor to fire */
      TrackSensor_t *se = &node->d.se;
      if (se->edges[0].dest == prev_idx) {
        reply->position = node->id * 2;
      } else {
        reply->position = node->id * 2 + 1;
      }
      reply->type = NODE_SENSOR;
      break;

    } else if (node->type == NODE_STOP) {
      /* dead end */
      reply->type = NODE_STOP;
      reply->position = node->id;
      break;

    } else if (node->type == NODE_SWITCH) {
      TrackSwitch_t *sw = &node->d.sw;
      if (sw->behind.dest == prev_idx) {
        /* traverse the switch */
        prev_idx = next_dest;
        next_dest = sw->ahead[sw->direction].dest;
        reply->distance += sw->ahead[sw->direction].distance;

      } else {
        /* arrived at a split */
        reply->type = NODE_SWITCH;
        reply->position = node->id;
        reply->sw_state = (sw->ahead[SWITCH_CURVED].dest == prev_idx)
                            ? SWITCH_CURVED : SWITCH_STRAIGHT;
        break;

      }

    }
  }
}

static void GetNextSensor (TrackQuery_t *q, TrackQueryReply_t *reply)
{
  for (;;) {
    GetNext(q, reply);
    if (reply->type == NODE_SENSOR || reply->type == NODE_STOP) {
      break;
    }
    q->type = reply->type;
    q->id = reply->position;
  }
}

static int IsOnPath (int sensor, int prev_idx, TrackEdge_t *edge)
{
  TrackNode_t *node = &track->nodes[edge->dest];
  if (node->type == NODE_SENSOR) {
    TrackSensor_t *se = &node->d.se;
    if ((se->edges[0].dest == prev_idx && sensor == node->id*2+1) ||
        (se->edges[1].dest == prev_idx && sensor == node->id*2)) {
      /* found the sensor */
      return edge->distance;
    }
    return 0;

  } else if (node->type == NODE_STOP) {
    /* reached a dead end */
    return 0;

  } else if (node->type == NODE_SWITCH) {
    TrackSwitch_t *sw = &node->d.sw;
    if (sw->behind.dest == prev_idx) {
      /* traverse the switch in both directions*/
      int cdist = IsOnPath(sensor, edge->dest, &sw->ahead[SWITCH_CURVED]);
      if (cdist) {
        return edge->distance + cdist;
      } else {
        int sdist = IsOnPath(sensor, edge->dest, &sw->ahead[SWITCH_STRAIGHT]);
        if (sdist) {
          return edge->distance + sdist;
        }
      }

    } else {
      /* pass through the merge */
      int mdist = IsOnPath(sensor, edge->dest, &sw->behind);
      if (mdist) {
        return edge->distance + mdist;

      }
    }
  }

  return 0;
}

static int UpdatePath (int sensor, int prev_idx, TrackEdge_t *edge, int exec)
{
  TrackNode_t *node = &track->nodes[edge->dest];
  if (node->type == NODE_SENSOR) {
    TrackSensor_t *se = &node->d.se;
    if ((se->edges[0].dest == prev_idx && sensor == node->id*2+1) ||
        (se->edges[1].dest == prev_idx && sensor == node->id*2)) {
      /* found the sensor */
      return 1;
    }
    return 0;

  } else if (node->type == NODE_STOP) {
    /* reached a dead end */
    return 0;

  } else if (node->type == NODE_SWITCH) {
    TrackSwitch_t *sw = &node->d.sw;
    if (sw->behind.dest == prev_idx) {
      /* traverse the switch in both directions*/
      if (UpdatePath(sensor, edge->dest, &sw->ahead[SWITCH_CURVED], exec)) {
        TrackSwitchState_t swst = { node->id, SWITCH_CURVED, exec };
        TrackSwitch(&swst);
        return 1;
      } else if (UpdatePath(sensor, edge->dest, &sw->ahead[SWITCH_STRAIGHT], exec)) {
        TrackSwitchState_t swst = { node->id, SWITCH_STRAIGHT, exec };
        TrackSwitch(&swst);
        return 1;
      }

    } else {
      /* pass through the merge */
      int mdist = UpdatePath(sensor, edge->dest, &sw->behind, exec);
      if (mdist) {
        return 1;
      }
    }
  }

  return 0;
}

static int GetEdge (TrackQuery_t *q, TrackEdge_t **e)
{
  if (q->type == NODE_SENSOR) {
    int se_index = track->sensor_nodes[q->id];
    TrackSensor_t *sensor = &track->nodes[se_index].d.se;
    *e = &sensor->edges[q->id % 2];
    return se_index;

  } else if (q->type == NODE_SWITCH) {
    int sw_index = track->switch_nodes[q->id];
    TrackSwitch_t *sw = &track->nodes[sw_index].d.sw;
    *e = &sw->ahead[sw->direction];
    return sw_index;

  } else if (q->type == NODE_STOP) {
    int st_index = track->stop_nodes[q->id];
    TrackStop_t *st = &track->nodes[st_index].d.st;
    *e = &st->ahead;
    return st_index;
    
  } else {
    *e = 0;
    return -1;

  }
}

static void TrackAddObserver (TrackObserver_t *obs)
{
  TrackCommand_t cmd;
  cmd.type = TRACK_OBSERVER;
  cmd.c.obs = *obs;
  /* just queue the event */
  QueueEvent(&cmd);
}

static void QueueEvent (TrackCommand_t *cmd)
{
  /* queue the event if we have room */
  if (eventq.size < EVENTQ_MAX) {
    eventq.cmds[ eventq.tail ] = *cmd;
    eventq.tail = (eventq.tail + 1) % EVENTQ_MAX;
    eventq.size = (eventq.size + 1);
    /* if we have a waiting courier, broadcast */
    if (courier_pid) {
      BroadcastEvents(courier_pid);
      courier_pid = 0;
    }
  } else {
    printf("track: event queue overflow\n");
  }
}

/* Sends events to our observers via the courier. */
static void BroadcastEvents (int courier)
{
  if (eventq.size > 0) {
    /* pull an event out of the queue */
    TrackCommand_t cmd = eventq.cmds[ eventq.head ];
    eventq.head = (eventq.head + 1) % EVENTQ_MAX;
    eventq.size = (eventq.size - 1);
    /* send it to the courier */
    Reply(courier, (char *)&cmd, sizeof(cmd));
  } else {
    /* wait until we have an event to send */
    courier_pid = courier;
  }
}

static void InitPathfinding ()
{
  /* initialize the path predicates */
  for (int i = 0; i < SENSORS_MAX; i++) {
    for (int j = 0; j < SENSORS_MAX; j++) {
      path_pred_model[i][j] = i;
      path_disconnections[i][j] = 0;
    }
  }

  /* initialize the distance matrix */
  for (int i = 0; i < SENSORS_MAX; i++) {
    TrackQuery_t q = { NODE_SENSOR, i };
    TrackEdge_t *e;

    /* query for adjacent nodes*/
    int start_idx = GetEdge(&q, &e);
    for (int s = 0; s < SENSORS_MAX; s++) {
      /* connect to itself at zero distance */
      if (s == i) {
        path_dist_model[i][i] = 0;
        continue;
      }
      /* determine connectivity */
      int dist = IsOnPath(s, start_idx, e);
      if (dist) {
        path_dist_model[i][s] = dist;
      } else {
        path_dist_model[i][s] = MAX_DISTANCE;
      }
    }

    /* connect to the opposite sensor at zero distance */
    path_dist_model[i][TRACK_OPPOSITE_SENSOR(i)] = 0;
  }

  /* run the pathfinder on the graph */
  UpdatePathfinding(0);
}

static void UpdatePathfinding (TrackPathModification_t *pm)
{
  /* copy the good model */
  memcpy(path_dist, path_dist_model, SENSORS_MAX * SENSORS_MAX * sizeof(int));
  memcpy(path_pred, path_pred_model, SENSORS_MAX * SENSORS_MAX * sizeof(int));

  /* update the disconnections */
  if (pm) {
    TrackQuery_t q = { NODE_SENSOR, pm->sensor };
    TrackEdge_t *e;
    int start_idx = GetEdge(&q, &e);
    for (int s = 0; s < SENSORS_MAX; s++) {
      int dist = IsOnPath(s, start_idx, e);
      if (dist) {
        path_disconnections[pm->sensor][s] = pm->disconnect;
        path_disconnections[TRACK_OPPOSITE_SENSOR(s)][TRACK_OPPOSITE_SENSOR(pm->sensor)] = pm->disconnect;
      }
    }

    /* disconnect itself */
    path_disconnections[pm->sensor][TRACK_OPPOSITE_SENSOR(pm->sensor)] = pm->disconnect;
    path_disconnections[TRACK_OPPOSITE_SENSOR(pm->sensor)][pm->sensor] = pm->disconnect;
  }

  /* initialize the distance matrix */
  for (int i = 0; i < SENSORS_MAX; i++) {
    for (int j = 0; j < SENSORS_MAX; j++) {
      if (path_disconnections[i][j]) {
         path_dist[i][j] = MAX_DISTANCE;
      }
    }
  }
 
  /* run Floyd-Warshall */
  for (int k = 0; k < SENSORS_MAX; k++) {
    for (int i = 0; i < SENSORS_MAX; i++) {
      for (int j = 0; j < SENSORS_MAX; j++) {
        if (path_dist[i][j] > path_dist[i][k] + path_dist[k][j]) {
          path_dist[i][j] = path_dist[i][k] + path_dist[k][j];
          path_pred[i][j] = path_pred[k][j];
        }
      }
    }
  }
}

static void TrackFindPath (int req_pid, TrackPathfindRequest_t *pr)
{
  TrackPath_t path_result;

  /* find the start and end points */
  int start, end, offset;
  GetPathEnd(pr, &end, &offset);
  GetPathStart(pr, end, &start);

  /* generate the path */
  path_result.size = 0;
  for (int cur = end; ; cur = path_pred[start][cur]) {
    float dist = path_dist[cur][end] + offset;
    if (dist < 0 && cur != end) {
      dist = -dist;
    }
    path_result.path[path_result.size].dist_remaining = dist;
    path_result.path[path_result.size].sensor = cur;
    path_result.size += 1;
    if (cur == start) {
      break;
    }
  }
  if (path_result.path[path_result.size - 1].dist_remaining >= MAX_DISTANCE) {
    path_result.size = 0;
  }
 
  Reply(req_pid, (char *)&path_result, sizeof(int) + path_result.size * sizeof(TrackPathStep_t));
}

static void GetPathStart (TrackPathfindRequest_t *pr, int end, int *start)
{
  /* find the optimal initial sensor */
  int fwd_dist = (pr->start_fwd >= 0)
    ? path_dist[pr->start_fwd][end] + pr->start_fwd_dist : MAX_DISTANCE;
  int rev_dist = (pr->start_rev >= 0)
    ? path_dist[pr->start_rev][end] + pr->start_rev_dist : MAX_DISTANCE;
  *start = (fwd_dist < rev_dist) ? pr->start_fwd : pr->start_rev;
}

static void GetPathEnd (TrackPathfindRequest_t *pr, int *end, int *offset)
{
  switch (pr->end_type) {
    case NODE_SENSOR: {
      *end = pr->end;
      *offset = pr->end_offset;
      break;
    }

    case NODE_SWITCH: {
      TrackQuery_t q = { NODE_SWITCH, pr->end };
      TrackQueryReply_t r = { 0, };

      TrackSwitch_t *sw = &track->nodes[track->switch_nodes[pr->end]].d.sw;
      /* fake the direction for the query */
      int old_dir = sw->direction;
      sw->direction = pr->end_sw_state;
      GetNextSensor(&q, &r);
      /* and restore it */
      sw->direction = old_dir;

      *end = r.position;
      *offset = pr->end_offset - r.distance;
      break;
    }

    case NODE_STOP: {
      TrackQuery_t q = { NODE_STOP, pr->end };
      TrackQueryReply_t r = { 0, };

      GetNextSensor(&q, &r);

      *end = TRACK_OPPOSITE_SENSOR(r.position);
      *offset = r.distance - pr->end_offset;
      break;
    }
  }
}

