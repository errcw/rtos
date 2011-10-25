/*
 * Multiboot-based kernel startup code.
 */
#define ASM
#include "multiboot.h"

/* Flags for the header. */
#define MULTIBOOT_HEADER_FLAGS (0x00000003)

/* Create a 16-kb stack */
#define STACK_SIZE (0x4000)

.globl _start, kernel_entry, stack_top
.text

multiboot_header:
  .long MULTIBOOT_HEADER_MAGIC
  .long MULTIBOOT_HEADER_FLAGS
  .long -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

_start:
  jmp multiboot_entry
  
multiboot_entry:
  finit
  /* initialize stack pointer */
  movl $(stack + STACK_SIZE), %esp

  /* reset EFLAGS */
  pushl $0
  popf

  /* push the pointer to the Multiboot information structure */
  pushl %ebx
  /* push the magic value */
  pushl %eax

  /* enter the C main function */
  call main

  /* halt after main exits */
loop:
  hlt
  jmp loop


  /* Our stack area. */
  .comm stack, STACK_SIZE

