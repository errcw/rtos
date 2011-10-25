/*
 * Schedules processes.
 *
 * Uses round robin priority-based scheduling.
 */
#ifndef __SCHEDULE_H
#define __SCHEDULE_H

#include "process.h"
#include "kern/limits.h"

/*
 * Initialses the scheduler.
 */
int ScheduleInit ();

/*
 * Schedules the next active process.
 */
Process_t *ScheduleGetNext ();

/*
 * Forces the next process selected by the scheduler.
 */
int ScheduleSetNext (Process_t *proc);

/*
 * Adds a process to be scheduled.
 */
int ScheduleAdd (Process_t *proc);

#endif

