/*
 * Manages memory. Handles the global descriptor table.
 */
#ifndef __MEMORY_H
#define __MEMORY_H

/* The top of memory is at 256MB. */
#define MEMORY_TOP (0x10000000)

/* Segment types. */
#define SEGMENT_RO  (0x10) /* read only */
#define SEGMENT_RW  (0x12) /* read write */
#define SEGMENT_ROD (0x14) /* read only expand dwn limit */
#define SEGMENT_RWD (0x16) /* read write expand dwn limit */
#define SEGMENT_E   (0x18) /* execute only */
#define SEGMENT_ER  (0x1a) /* execute read */
#define SEGMENT_EC  (0x1c) /* execute only conforming */
#define SEGMENT_ERC (0x1e) /* execute read conforming */

/* Kernel privilege level (ring 0). */
#define SEGMENT_RING0 (0)
/* User privilege level (ring 3). */
#define SEGMENT_RING3 (3)

/* The kernel segment descriptors. */
#define SEGMENT_KERNEL_CODE (8)
#define SEGMENT_KERNEL_DATA (16)

/* A segment entry in the GDT. */
typedef struct Segment {
  unsigned int start; /* the start address of the segment */
  unsigned int limit; /* the end address of the segment */
  unsigned int type; /* the segment type */
  unsigned int ring; /* the access rights */
} Segment_t;

/* Segment ids. */
typedef unsigned short SegmentId_t;

/*
 * Initializes memory. Adds GDT entries for the kernel and sets the segment registers appropriately.
 */
int MemoryInit ();

/*
 * Adds a segment to the GDT. Returns the id of the segment descriptor in the GDT.
 */
SegmentId_t MemoryCreateSegment (Segment_t *seg);

/*
 * Frees a segment in the GDT.
 */
int MemoryFreeSegment (SegmentId_t segid);

#endif

