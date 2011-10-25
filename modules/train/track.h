/*
 * The track model.
 */
#ifndef __TRACK_H
#define __TRACK_H

#include <trainlib.h>

/* The track model. */

#define TRACK_NODES_MAX (72)
#define TRACK_NAME_MAX (8)

#define TRACK_PATH_MAX (80)

/* Find the sensor pointing in the opposite direction. */
#define TRACK_OPPOSITE_SENSOR(s) ((s) % 2 ? s - 1: s + 1)

typedef struct TrackEdge {
  int dest; /* the next node */
  int distance; /* the distance in millimetres */
} TrackEdge_t;

typedef struct TrackSensor {
  TrackEdge_t edges[2]; /* odd and even */
} TrackSensor_t;

typedef struct TrackSwitch {
  TrackEdge_t ahead[2]; /* straight, curved */
  TrackEdge_t behind; /* the node behind */
  int direction; /* the current switch direction */
} TrackSwitch_t;

typedef struct TrackStop {
  TrackEdge_t ahead; /* first node after the end */
} TrackStop_t;

typedef struct TrackNode {
  int id;
  char name[TRACK_NAME_MAX];
  enum {
    NODE_SWITCH,
    NODE_SENSOR,
    NODE_STOP,
  } type;
  union {
    TrackSwitch_t sw;
    TrackSensor_t se;
    TrackStop_t st;
  } d;
} TrackNode_t;

typedef struct TrackModel {
  int num_nodes;
  TrackNode_t nodes[TRACK_NODES_MAX];

  int sensor_nodes[SENSORS_MAX]; /* map from sensor id to sensor node index */
  int switch_nodes[SWITCHES_MAX]; /* map from switch id to switch node index */
  int stop_nodes[STOPS_MAX]; /* map from stop id to stop node index */
} TrackModel_t;

/* The model interface. */

/* The track configuration. */
typedef struct TrackCfg {
  char type;
} TrackCfg_t;

/* Request to add a track observer. */
typedef struct TrackObserver {
  int pid;
} TrackObserver_t;

/* The sensor state. */
typedef struct TrackSensorState {
  struct {
    unsigned int lo;
    unsigned int hi;
  } modules[SENSOR_MODS_MAX];
} TrackSensorState_t;

/* The switch state. */
typedef struct TrackSwitchState {
  int swnum;
  int direction;
  int exec;
} TrackSwitchState_t;

/* A track query. */
typedef struct TrackQuery {
  int type;
  int id;
} TrackQuery_t;

/* A path query. */
typedef struct TrackPathQuery {
  int start;
  int end;
  int exec;
} TrackPathQuery_t;

/* A reply to a track query. */
typedef struct TrackQueryReply {
  int type;
  int position;
  int distance;
  int sw_state;
} TrackQueryReply_t;

/* A query for sensors along a given distance. */
typedef struct TrackSensorQuery {
  int start;
  float distance;
} TrackSensorQuery_t;

/* A reply for sensors along a given distance. */
typedef struct TrackSensorQueryReply {
  int size;
  int sensors[SENSORS_MAX];
} TrackSensorQueryReply_t;

/* A request to find a path on the track. */
typedef struct TrackPathfindRequest {
  int start_fwd;
  int start_fwd_dist;
  int start_rev;
  int start_rev_dist;
  int end;
  int end_type;
  int end_offset;
  int end_sw_state;
} TrackPathfindRequest_t;

/* One step in the overall path. */
typedef struct TrackPathStep {
  int sensor;
  int dist_remaining;
} TrackPathStep_t;

/* A path along the track. */
typedef struct TrackPath {
  int size;
  TrackPathStep_t path[TRACK_PATH_MAX];
} TrackPath_t;

/* Track disconnections */
typedef struct TrackPathModification {
  int disconnect;
  int sensor;
} TrackPathModification_t;

/* A track command. */
typedef struct TrackCommand {
  enum {
    TRACK_CONFIG,
    TRACK_OBSERVER,
    TRACK_SENSOR,
    TRACK_SWITCH,
    TRACK_COURIER,
    TRACK_GETNEXT,
    TRACK_GETNEXT_INVERTED,
    TRACK_GETNEXT_SENSOR,
    TRACK_GETNEXT_SENSORS,
    TRACK_ISONPATH,
    TRACK_UPDATEPATH,
    TRACK_GETPATH,
    TRACK_MODPATH
  } type;
  union {
    TrackCfg_t cfg;
    TrackObserver_t obs;
    TrackSensorState_t sens;
    TrackSwitchState_t sw;
    TrackQuery_t q;
    TrackSensorQuery_t sq;
    TrackPathQuery_t pq;
    TrackPathfindRequest_t path;
    TrackPathModification_t pm;
  } c;
} TrackCommand_t;

#endif

