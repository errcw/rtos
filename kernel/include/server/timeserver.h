/*
 * Interface for communicating with the timer server.
 */

#ifndef __TIMESERVER_H
#define __TIMESERVER_H

/* The name used by the time server to register itself. */
#define TIMESERVER_NAME ("ts")

/* The timer priority. */
#define TIMESERVER_PRIORITY (15)

/* The default frequency of ticking. */
#define TIMESERVER_HZ (1000)

/* Request code to increase counter. */
#define TIMESERVER_TICK (-1)
/* Request code to get the current time. */
#define TIMESERVER_GETTIME (-2)

typedef struct TimeNotifierCfg {
  int hz; /* clock frequency */
  int courier; /* courier pid */
} TimeNotifierCfg_t;

#endif

