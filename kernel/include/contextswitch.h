/*
 * Manages entering and leaving the kernel.
 */
#ifndef __CONTEXTSWITCH_H
#define __CONTEXTSWITCH_H

/*
 * Entry point to the kernel from a user process.
 */
void KernelEnter ();

/*
 * Exit point from the kernel to a user process.
 */
void KernelLeave ();

#endif
