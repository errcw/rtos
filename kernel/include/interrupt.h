/*
 * Interrupt descriptor table manipulation.
 */
#ifndef __IDT_H
#define __IDT_H

/* Interrupt types. */
#define INTERRUPT_DIVIDEERROR (0) /* divide error */
#define INTERRUPT_DEBUG (1) /* debug exception */
#define INTERRUPT_NMI (2) /* non-maskable interrupt */
#define INTERRUPT_BREAK (3) /* breakpoint exception (one-byte INT 3 instruction) */
#define INTERRUPT_OVERFLOW (4) /* overflow (INTO instruction) */
#define INTERRUPT_BOUNDS (5) /* bounds check (BOUND instruction) */
#define INTERRUPT_INVALIDOPCODE (6) /* invalid opcode */
#define INTERRUPT_COPROCNOTAVAIL (7) /* coprocessor not available */
#define INTERRUPT_DOUBLEFAULT (8) /* double fault */
#define INTERRUPT_INVALIDTSS (10) /* invalid TSS */
#define INTERRUPT_SEGNOTPRES (11) /* segment not present */
#define INTERRUPT_STACK (12) /* stack exception */
#define INTERRUPT_GENERALPROTECTION (13) /* general protection */
#define INTERRUPT_PAGEEAULT (14) /* page fault */
#define INTERRUPT_COPROCERROR (15) /* coprocessor error */

/* The maximum number of interrupts. */
#define INTERRUPT_MAX (256)

/* The interrupt number start. */ 
#define IRQ_BASE (32)
/* The maximum IRQ number. */
#define IRQ_NUM (16)

/* Interrupt handler function pointer. */
typedef void (*InterruptHandler_t)(unsigned int);

/*
 * Initializes the IDT.
 */
int InterruptInit ();

/*
 * Sets the kernel function to execute when the given interrupt fires.
 */
int InterruptHook (unsigned int interrupt, InterruptHandler_t handler);

/*
 * Removes the function associated with the given interrupt.
 */
int InterruptUnhook (unsigned int interrupt);

/*
 * Executes a function in the kernel based on the last interrupt to fire.
 */
int InterruptDispatch ();

#endif

