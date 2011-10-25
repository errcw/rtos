/*
 * Context switch.
 */

  .file "contextswitch.S"
  .text
  .align 4

  /* The kernel stack pointer. */
  .comm kernelsp, 4

.globl KernelEnter
KernelEnter:
  /* save the user state */
  pushl %eax
  pushl %ebx
  pushl %ecx
  pushl %edx
  pushl %esi
  pushl %edi
  pushl %ebp
  pushw %ds
  pushw %es
  pushw %fs
  pushw %gs
  fsave -108(%esp)

  /* restore the kernel's segment selectors */
  movl $16, %ebx
  movw %bx, %ss
  movw %bx, %ds
  movw %bx, %es
  movw %bx, %fs
  movw %bx, %gs

  /* store the user stack */
  movl activeproc, %eax
  movl %esp, 0(%eax)

  /* retrieve the kernel stack */
  movl (kernelsp), %esp

  /* restore the kernel state */
  popl %ebp
  popl %edi
  popl %esi
  popl %edx
  popl %ecx
  popl %ebx
  popl %eax

  /* return to whence we came */
  ret

.globl KernelLeave
KernelLeave:
  /* save the kernel state */
  pushl %eax
  pushl %ebx
  pushl %ecx
  pushl %edx
  pushl %esi
  pushl %edi
  pushl %ebp

  /* store kernel stack */
  movl %esp, (kernelsp)

  /* get the user stack */
  movl activeproc, %ebx
  movl 0(%ebx), %esp
  movw 4(%ebx), %ss

  /* restore the user state */
  frstor -108(%esp)
  popw %gs
  popw %fs
  popw %es
  popw %ds
  popl %ebp
  popl %edi
  popl %esi
  popl %edx
  popl %ecx
  popl %ebx
  popl %eax

  addl $4, %esp /* ignore the interrupt number */
  
  /* return */
  iretl 

