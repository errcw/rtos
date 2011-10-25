#include "schedule.h"

/* The number of priority levels. */
#define PRIORITY_NUM (PRIORITY_MAX - PRIORITY_MIN + 1)

/* The maximum number of processes per priority. */
#define PRIORITY_PROCESS_MAX (8)

/* The round-robin queues at each priority. */
static struct {
  Process_t *entries[PRIORITY_PROCESS_MAX];
  unsigned int head;
  unsigned int tail;
  unsigned int size;
} schedule[PRIORITY_NUM];

/* A bitmap indicating the position of processes at each priority level. */
static unsigned int priority_map;

/* The forced next process. */
static Process_t *procnext;

/* Adds the process to the end of the queue for the given priority. */
static int AddQ (Process_t *proc, int pri);
/* Removes the process at the start of the queue for the given priority. */
static Process_t *RemoveQ(int pri);

/* Returns the highest priority with processes ready to run. */
static int FindHighestPriority ();

int ScheduleInit ()
{
  for (int pri = 0; pri < PRIORITY_NUM; pri++) {
    schedule[pri].head = 0;
    schedule[pri].tail = 0;
    schedule[pri].size = 0;
  }
  priority_map = 0;
  return 1;
}

Process_t *ScheduleGetNext ()
{
  /* override the scheduling algorithm */
  if (procnext) {
    Process_t *active = procnext;
    procnext = 0;
    active->state = ACTIVE;
    return active;
  }

  /* find the next ready process in the highest priority queue */
  int pri;
  while ((pri = FindHighestPriority()) >= 0) {
    Process_t *active = RemoveQ(pri);
    if (active->state == READY) {
      active->state = ACTIVE;
      return active;
    }
  }

  return 0;
}

int ScheduleSetNext (Process_t *proc)
{
  proc->state = READY;
  procnext = proc;
  return 1;
}

int ScheduleAdd (Process_t *proc)
{
  proc->state = READY;
  return AddQ(proc, proc->priority);
}

static int AddQ (Process_t *proc, int pri)
{
  /* ensure there is room */
  if (schedule[pri].size >= PRIORITY_PROCESS_MAX) {
    return 0;
  }
  /* add to the tail of the queue */
  schedule[pri].entries[ schedule[pri].tail ] = proc;
  schedule[pri].tail = (schedule[pri].tail + 1) % PRIORITY_PROCESS_MAX;
  schedule[pri].size = (schedule[pri].size + 1);
  priority_map |= (1 << pri);
  return 1;
}

static Process_t *RemoveQ (int pri)
{
  /* ensure there is a process to remove */
  if (schedule[pri].size == 0) {
    return 0;
  }
  /* pop off the head of the queue */
  Process_t *head = schedule[pri].entries[ schedule[pri].head ];
  schedule[pri].head = (schedule[pri].head + 1) % PRIORITY_PROCESS_MAX;
  schedule[pri].size = (schedule[pri].size - 1);
  /* if the queue is now empty, update our map */
  if (schedule[pri].size == 0) {
    priority_map &= ~(1 << pri);
  }
  return head;
}

static int FindHighestPriority ()
{
  static const signed char logtable256[] = {
   -1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
  };
  register unsigned int t, tt;
  if ((tt = priority_map >> 16)) {
    return (t = priority_map >> 24) ? 24 + logtable256[t] : 16 + logtable256[tt & 0xff];
  } else {
    return (t = priority_map >> 8) ? 8 + logtable256[t] : logtable256[priority_map];
  }
}

