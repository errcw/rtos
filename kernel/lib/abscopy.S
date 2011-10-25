/*
#
# Functions to copy from/to absolution locations in memory.
#
# Tries to be faster, by aligning its accesses and doing word moves
# which move 32 bits at a time.
#
*/

  .file  "abscopy.S"

  .text

/*# void AbsRead(char *to, unsigned int from, int count)*/
  .align 4
  .globl AbsRead
AbsRead:
  pushl %ebp
  movl %esp, %ebp

  pushl %ecx
  pushl %edx
  pushl %edi
  pushl %esi
  pushl %ds
  pushl %es

  /* get ready for a movsb */
  movl 8(%ebp), %edi  /* to */
  movl 12(%ebp), %esi /* from */
  movl 16(%ebp), %edx /* count */

  movl $0x10, %eax /* set DS = linear address space selector */
  movw %ax, %ds

  movw %ss, %ax /* set ES = local data segment */
  movw %ax, %es

  /* perform the move, quickly */
  call fast_copy

  popl %es
  popl %ds
  popl %esi  
  popl %edi  
  popl %edx
  popl %ecx

  movl %ebp, %esp
  popl %ebp
  ret

/*# void AbsWrite(unsigned int to, char *from, int count)*/
  .align 4
  .globl AbsWrite
AbsWrite:
  pushl %ebp
  movl %esp, %ebp

  pushl %ecx
  pushl %edx
  pushl %edi
  pushl %esi
  pushl %es

  /* get ready for a movsb */
  movl 8(%ebp), %edi  /* to */
  movl 12(%ebp), %esi /* from */
  movl 16(%ebp), %edx /* count */

  movl $0x10, %eax /* set ES = linear address space selector */
  movw %ax, %es

  /* perform the move, quickly. */
  call fast_copy

  popl %es
  popl %esi
  popl %edi
  popl %edx
  popl %ecx

  movl %ebp, %esp
  popl %ebp
  ret

/*# void AbsCopy(unsigned int to, unsigned int from, int count) */
  .align 4
  .globl AbsCopy
AbsCopy:
  pushl %ebp
  movl %esp, %ebp

  pushl %ecx
  pushl %edx
  pushl %edi
  pushl %esi
  pushl %ds
  pushl %es

  /* get ready for a movsb */
  movl 8(%ebp), %edi  /* to */
  movl 12(%ebp), %esi /* from */
  movl 16(%ebp), %edx /* count */

  movl $0x10, %eax /* set DS = linear address space selector */
  movw %ax, %ds
  movw %ax, %es    /* and ES = DS */

  /* perform the move, quickly */
  call fast_copy

  popl %es
  popl %ds
  popl %esi  
  popl %edi  
  popl %edx
  popl %ecx

  movl %ebp, %esp
  popl %ebp
  ret

/*
#
# Private function: call with DS:ESI and ES:EDI setup correctly and
#   edx loaded with the count in bytes.
#
# Damages: ecx, eax
#*/
fast_copy:
  /* If the string is too short, it's not worth the overhead of aligning
   * to word boundaries, etc. We jump to the plain byte copy instead.
   */
  cmpl $16, %edx
  jb 1f

  /* zero beginning until aligned */
  movl %edi, %ecx
  negl %ecx
  andl $3, %ecx
  subl %ecx, %edx
  rep
  movsb

  /* perform the aligned copy */
  movl %edx, %ecx /* edx = # qwords + # remainder bytes */
  shrl $2, %ecx   /* compute # qwords */
  andl $3, %edx   /* # remaining bytes */
  rep
  movsl

  /* do the final misaligned bytes */
1:
  movl %edx, %ecx
  rep
  movsb

  ret


/*** contributed code ****/
/*
From undergrad.math.uwaterloo.ca!bsdealwi Sun Jun 18 01:43:41 1995
From: Brian de Alwis <bsdealwi@undergrad.math.uwaterloo.ca>
To: pdgray@undergrad.math.uwaterloo.ca (Peter D. Gray)
Date: Sun, 18 Jun 1995 01:43:18 -0400

To convince you that the ugly cludges in crt0 aren't needed,
you will find a copy of absZero appended here. It's basically
a slightly modified version of NetBSD's bzero that behaves
like your abs{Write,Read,Copy} commands. (It's not quite exactly
the same as bzero, as NetBSD has some special-case code for the
i486 that I didn't understand).

Use: void absZero(unsigned int linear_address, unsigned int numBytes);
*/

/*
#
# Function to zero out memory
#
# This is written to be really fast. It was taken from NetBSDs locore.s
#*/

/*# void AbsZero(unsigned int start, int count) */
  .align 4
  .globl AbsZero
AbsZero:
  pushl %ebp
  movl %esp, %ebp

  pushl %ecx
  pushl %edx
  pushl %edi  
  pushl %es

  /* edx contains the number of chars left
   * edi is the current location
   * ecx is the number of elements about to be zeroed
   * es  is the absolute segment
   */
  movl 8(%ebp), %edi /* location */
  movl 12(%ebp), %edx /* count */

  movl $0x10, %eax /* set ES = linear address space sel */
  movw %ax, %es

  cld
  xorl %eax, %eax

  /* If the string is too short, it's not worth the overhead of aligning
   * to word boundaries, etc. We jump to the plain byte copy instead. */
  cmpl $16, %edx
  jb 7f

  /* zero beginning until aligned */
  movl %edi, %ecx
  negl %ecx
  andl $3, %ecx
  subl %ecx, %edx
  rep
  stosb

  /* perform the aligned zero */
  movl %edx, %ecx /* edx = # qwords + # remainder bytes */
  shrl $2, %ecx   /* compute # qwords */
  andl $3, %edx   /* # remaining bytes */
  rep
  stosl

  /* do the final misaligned bytes */
7:
  movl   %edx,%ecx
  rep
  stosb

  popl %es
  popl %edi
  popl %edx
  popl %ecx

  movl %ebp, %esp
  popl %ebp
  ret

