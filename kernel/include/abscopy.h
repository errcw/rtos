#ifndef __ABSCOPY_H
#define __ABSCOPY_H

/*
 * Copy count bytes from an absolute linear address in memory to the local address space.
 */
void AbsRead (void *to, unsigned int from, int count);

/*
 * Copy count bytes from the local address space to an absolute linear address in memory.
 */
void AbsWrite (unsigned int to, void *from, int count);

/*
 * Copy count bytes from one absolute linear address in memory to another.
 */
void AbsCopy (unsigned int to, unsigned int from, int count); /* .. or from-to abs loc */

/*
 * Zero count bytes at the given absolute linear memory address.
 */
void AbsZero (unsigned int to, int count);

#endif

