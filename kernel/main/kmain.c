#include "lib.h"
#include "multiboot.h"
#include "video.h"
#include "memory.h"
#include "process.h"
#include "module.h"
#include "contextswitch.h"
#include "schedule.h"
#include "interrupt.h"
#include "ksyscall.h"
#include "kern/limits.h"

int main (unsigned long magic, unsigned long mbiaddr)
{
  /* check our magic value is magic */
  if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
    return 1;
  }

  multiboot_info_t *mbi = (multiboot_info_t *)mbiaddr;

  /* initialize the kernel segments */
  if (!MemoryInit()) {
    return 2;
  }

  /* initialize the video memory */
  VideoInit();

  kprintf("Core is...go!\n");

  /* initialize the interrupts */
  kprintf("Interrupts...");
  if (!InterruptInit()) {
    kprintf("failed.\n");
    return 0;
  }
  kprintf("go!\n");

  /* initialize the modules */
  kprintf("Modules...");
  if (!ModuleInit((module_t *)mbi->mods_addr, mbi->mods_count)) {
    kprintf("failed.\n");
    return 0;
  }
  kprintf("go!\n");

  /* initialize processes */
  kprintf("Processes...");
  if (!ProcessInit()) {
    kprintf("failed.\n");
    return 0;
  }
  kprintf("go!\n");

  /* initialize the scheduler */
  kprintf("Scheduler...");
  if (!ScheduleInit()) {
    kprintf("failed.\n");
    return 0;
  }
  kprintf("go!\n");

  /* initialize the system calls */
  kprintf("Syscalls...");
  if (!SyscallInit()) {
    kprintf("failed.\n");
    return 0;
  }
  kprintf("go!\n");

  /* initialize the bootstrap process */
  kprintf("Bootstrap process...");
  ModuleInfo_t *mod_info = ModuleFind("init");
  Process_t *proc = ProcessCreate(mod_info);
  if (!proc) {
    kprintf("failed.\n");
    return 0;
  }
  proc->priority = PRIORITY_MAX;
  proc->parent_pid = 0;
  if (!ScheduleAdd(proc)) {
    kprintf("failed.\n");
    return 0;
  }
  activeproc = 0;
  kprintf("go!\n");

  kprintf("Kernel is...GO!\n\n");

  /* run the user processes */
  while ((activeproc = ScheduleGetNext())) {
    KernelLeave();
    InterruptDispatch();
    if (activeproc->state == ACTIVE) {
      ScheduleAdd(activeproc);
    }
  }

  kprintf("\nGood bye!\n");

  /* return something to know we halted in the right place */
  return 0x00facade;
}

