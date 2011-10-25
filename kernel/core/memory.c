#include "memory.h"
#include "contextswitch.h"

/* The maximum number of entries in the GDT. */
#define GDT_MAX (128)

#pragma pack(1) /* THESE STRUCTURES MUST BE PACKED */

/* The pointer to the GDT. */
typedef struct GDTPointer {
  unsigned int limit : 16;
  unsigned int base : 32;
} GDTPointer_t;

/* A GDT segment descriptor. */
typedef struct SegmentDesc {
  unsigned int limitlo : 16;
  unsigned int baselo : 24;
  unsigned int type : 5; /* segment type */
  unsigned int dpl : 2; /* descriptor privilege level */
  unsigned int present : 1; /* segment present in memory */
  unsigned int limithi : 4;
  unsigned int unused : 1;
  unsigned int zero : 1; /* always zero */
  unsigned int opsize : 1; /* operand size; 0 is 16bit, 1 is 32bit */
  unsigned int granularity : 1; /* granularity; 0 is 1 byte, 1 is 1 kbyte */
  unsigned int basehi : 8;
} SegmentDesc_t;

/* The GDT. */
static SegmentDesc_t gdt[GDT_MAX];

/* The GDT free list. */
static unsigned int gdt_free[GDT_MAX];
static int gdt_free_top;

/* Sets the GDT index to the given segment. */
static void MemorySetSegment (int gdtidx, Segment_t *seg);

/* Sets up the GDT. */
static void SetGDT (GDTPointer_t *ptr);


int MemoryInit ()
{
  GDTPointer_t gdtptr;
  gdtptr.base = (unsigned int)&gdt;
  gdtptr.limit = sizeof(gdt) - 1;

  /* invalidate all the entries */
  for (int i = 0; i < GDT_MAX; i++) {
    gdt[i].present = 0;
    gdt_free[i] = i + 1;
  }

  /* add the kernel entries */
  Segment_t kseg;
  kseg.start = 0;
  kseg.limit = MEMORY_TOP;
  kseg.ring = SEGMENT_RING0;
  kseg.type = SEGMENT_ER;
  MemorySetSegment(SEGMENT_KERNEL_CODE >> 3, &kseg); /* kernel code */
  kseg.type = SEGMENT_RW;
  MemorySetSegment(SEGMENT_KERNEL_DATA >> 3, &kseg); /* kernel data */

  gdt_free_top = 2;

  /* point to the GDT */
  SetGDT(&gdtptr);

  /* success! */
  return 1;
}

SegmentId_t MemoryCreateSegment (Segment_t *seg)
{
  /* check if we have any segments left */
  if (gdt_free_top >= GDT_MAX) {
    return 0;
  }
  /* allocate and initialize segment */
  unsigned int index = gdt_free[gdt_free_top++];
  MemorySetSegment(index, seg);
  /* return the segment selector (index*8) */
  return (index << 3);
}

int MemoryFreeSegment (SegmentId_t segid)
{
  /* check for a valid GDT index */
  int gdtidx = segid >> 3;
  if (gdtidx <= 0 || gdtidx > GDT_MAX) {
    return 0;
  }
  SegmentDesc_t *entry = &gdt[gdtidx];
  /* check for a valid descriptor */
  if (!entry->present) {
    return 0;
  }
  /* invalidate the GDT entry */
  entry->present = 0;
  gdt_free[--gdt_free_top] = gdtidx;
  return 1;
}

static void MemorySetSegment (int segidx, Segment_t *seg)
{
  gdt[segidx].baselo = ((seg->start >> 0) & 0xffffff);
  gdt[segidx].basehi = ((seg->start >> 24) & 0xff);

  gdt[segidx].limitlo = ((seg->limit >> 12) & 0xffff);
  gdt[segidx].limithi = ((seg->limit >> 28) & 0x0f);

  gdt[segidx].type = seg->type;
  gdt[segidx].dpl = seg->ring;
  gdt[segidx].granularity = 1; /* always 1kb granularity */
  gdt[segidx].opsize = 1; /* always 32-bit ops */

  gdt[segidx].present = 1;
}

static void SetGDT (GDTPointer_t *ptr)
{
  __asm__ __volatile__
  (
  "lgdt (%[p]) \n" /* load the GDTR */

  "movw $16, %%ax \n" /* load the data segment selectors */
  "movw %%ax, %%ds \n"
  "movw %%ax, %%es \n"
  "movw %%ax, %%fs \n"
  "movw %%ax, %%gs \n"
  "movw %%ax, %%ss \n"

  "ljmp $8, $farjmp \n" /* load the code segment selector */
  "farjmp: nop \n"

  : : [p] "r" (ptr));
}

