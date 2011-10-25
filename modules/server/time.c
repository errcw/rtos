/*
 * The time server.
 */
#include <stdio.h>
#include <string.h>

#include <timeserver.h>
#include <syscall.h>
#include <limits.h>

#define NOTIFIER_PRIORITY (TIMESERVER_PRIORITY + 1)

#define WAITERS_MAX (32)

/* The current time. */
static int curmin;
static int cursec;
static int curms;

/* The waiting processes. */
static struct {
  int pid;
  int remaining_delay;
} waiters[WAITERS_MAX];

/* Tracks free space in the waiters array. */
static int free_waiter_slots[WAITERS_MAX];
static int free_top;

/* Handles a timer tick. */
static void TimerTick ();

/* Adds a process to wait. */
static int AddDelay (int pid, int delay);

/* Set up and run the server. */
static void Init ();
static void Run ();

int main ()
{
  Init();
  Run();
  Exit();
}

static void Init ()
{
  curms = 0;
  cursec = 0;
  curmin = 0;

  /* initialize the waiters */
  for (int i = 0; i < WAITERS_MAX; i++) {
    waiters[i].pid = 0;
    free_waiter_slots[i] = i;
  }
  free_top = 0;

  /* register ourselves */
  if (!RegisterAs(TIMESERVER_NAME)) {
    printf("RegisterAs failed!\n");
  }

  /* create our notifier and courier */
  int high_pri = MyPriority() + 1;
  int notifier_pid = Create("time_notifier", high_pri);
  if (notifier_pid <= 0) {
    printf("time server: Create notifier failed (%s)\n", strerror(notifier_pid));
    Exit();
  }
  int courier_pid = Create("time_courier", high_pri);
  if (courier_pid <= 0) {
    printf("time server: Create courier failed (%s)\n", strerror(courier_pid));
    Exit();
  }

  /* initialize them */
  int result;
  result = Send(courier_pid, 0, 0, 0, 0);
  if (result < 0) {
    printf("time server: init Send to courier failed (%s)\n", strerror(result));
    Exit();
  }
  TimeNotifierCfg_t cfg = { TIMESERVER_HZ, courier_pid };
  result = Send(notifier_pid, (char *)&cfg, sizeof(cfg), 0, 0);
  if (result < 0) {
    printf("time server: init Send to notifier failed (%s)\n", strerror(result));
    Exit();
  }
}

static void Run ()
{
  int request;

  for (;;) {
    int pid = Receive((char *)&request, sizeof(request));
    if (pid > 0) {
      if (request > 0) { /* delay */
        if (AddDelay(pid, request) < 0) {
          Reply(pid, 0, 0); /* reply right away with failure */
        }
      } else if (request == TIMESERVER_GETTIME) { /* get time */
        Time_t time = { curms, cursec, curmin };
        Reply(pid, (char *)&time, sizeof(time));
      } else if (request == TIMESERVER_TICK) { /* tick */
        Reply(pid, 0, 0);
        TimerTick();
      }
    } else {
      printf("time server: Receive failed (%s)\n", strerror(pid));
    }
  }
}

static void TimerTick () 
{
  /* track the time */
  curms += 1;
  if (curms >= 1000) {
    curms = 0;
    cursec += 1;
    if (cursec >= 60) {
      cursec = 0;
      curmin += 1;
    }
  }

  /* service our delays */
  for (int i = 0; i < WAITERS_MAX; i++) {
    if (waiters[i].pid && --waiters[i].remaining_delay == 0) {
      /* free the slot */
      int pid = waiters[i].pid;
      waiters[i].pid = 0;
      free_waiter_slots[--free_top] = i;
      /* wake up the thread */
      int result = Reply(pid, 0, 0);
      if (result < 0) {
        printf("time server: could not wake up %d (%s)\n", pid, strerror(result));
      }
    }
  }
}

static int AddDelay (int pid, int delay)
{
  if (free_top >= WAITERS_MAX) {
    return -1;
  }
  int index = free_waiter_slots[free_top++];
  waiters[index].pid = pid;
  waiters[index].remaining_delay = delay;
  return 0;
}

