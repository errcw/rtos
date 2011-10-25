/*
 * Entry point for all user programs.
 *
 * We assume the stack contains (most recently pushed first):
 *
 * - the kernel data segment selector (with absolute memory addressing)
 * - the location where our initialized data is stored
 * - the number of bytes of initialized data we should copy to our DS
 * - the number of bytes of bytes to zero out
 */

  .file "crt0.S"
  .text
  .align 4

/*
 * Entry point known to the linker. The "e_entry" field in the ELF header points here.
 */
.globl _start
_start:

  xorl %edi, %edi /* we're going to copy address zero */
  popl %eax       /* data segment selector of kernel */
  movw %ax, %ds
  popl %esi       /* elf data segment to copy from */
  popl %ecx       /* number of bytes to copy */
  
  /* copy ECX bytes of data from DS:ESI to ES:EDI */
  cld
  rep
  movsb

  popl %ecx       /* number of bytes to clear */
  xorl %eax, %eax /* write zeros */

  /* fill zeros at ES:EDI, EDI is pointing at the end of the initialized data */
  rep
  stosb

  /* reset our DS */
  movw %es, %ax
  movw %ax, %ds
  
  /* jump to main (anything on the stack will go to the process main) */
  jmp main


/*
 * Exit point, called by exit() in the standard library.
 */
.globl __exit
__exit:

  call Exit  /* call the Exit() system call */
  jmp __exit /* should never reach here */

