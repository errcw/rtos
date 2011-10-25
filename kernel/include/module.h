/*
 * Handles finding and loading modules.
 */
#ifndef __MODULE_H
#define __MODULE_H

#include "multiboot.h"

/* The maximum number of modules supported by the kernel. */
#define MODULES_MAX (32)

typedef struct ModuleInfo {
  char *name; /* the module name */

  unsigned int module_start; /* the start of the ELF in memory */
  unsigned int module_end; /* the end of the ELF */

  unsigned int code_start; /* the start of the .text section */
  unsigned int code_size; /* the size of the .text section */
  unsigned int code_entry; /* the execution entry point */

  unsigned int data_start; /* the start of the data section */
  unsigned int data_init_size; /* the size of the initialized static data */
  unsigned int data_size; /* the size of the zero-initialized data */
} ModuleInfo_t;

/*
 * Initializes the list of modules in the system.
 */
int ModuleInit (module_t *mods, unsigned int mods_count);

/*
 * Finds a module by name
 */
ModuleInfo_t *ModuleFind (char *name);

#endif

