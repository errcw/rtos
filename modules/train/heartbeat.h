/*
 * Provides a regular heartbeat to a process.
 */

#ifndef __HEARTBEAT_H
#define __HEARTBEAT_H

/* The heartbeat message id. */
#define HEARTBEAT (0x800)

/* Heartbeat configuration. */
typedef struct HeartbeatCfg {
  int pid;
  int delay;
} HeartbeatCfg_t;

#endif

