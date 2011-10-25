#include <machine/cpufunc.h>
#include <machine/pio.h>

#include "interrupt.h"
#include "isr.h"
#include "contextswitch.h"
#include "ksyscall.h"
#include "lib.h"

/* Interrupt descriptor types. */
#define DESCTYPE_INT_TASK (0x05)
#define DESCTYPE_INT_INTERRUPT (0x0e)
#define DESCTYPE_INT_TRAP (0x0f)

/* Stack offset to find the interrupt number. */
#define INTERRUPT_NUM_OFFSET (9)

/* Interrupt controller hard codes. */
#define IO_ICU1 (0x020)   /* 8259A Interrupt Controller #1 */
#define IO_ICU2 (0x0A0)   /* 8259A Interrupt Controller #2 */
#define ICU_LEN (16)      /* 32-47 are ISA interrupts */
#define IRQ_SLAVE (0x0004)

/* Constants for IrqSet. */
#define IRQ_ENABLE (0)
#define IRQ_DISABLE (1)

/* A short delay to avoid overloading the 8259. */
static void BusyWait ();
/* Configure the interrupt controller units. */
static void Config8259s ();

#pragma pack(1)

/* The pointer to the IDT. */
typedef struct IDTPointer {
  unsigned int limit : 16;
  unsigned int base : 32;
} IDTPointer_t;

/* An interrupt gate descriptor. */
typedef struct IDTGateDesc {
  unsigned int offsetlo : 16; /* gate offset (lsb) */
  unsigned int selector : 16; /* gate segment selector */
  unsigned int unused : 8;
  unsigned int type : 5; /* descriptor type */
  unsigned int dpl : 2; /* descriptor privilege level */
  unsigned int present : 1; /* present in memory */
  unsigned int offsethi : 16; /* gate offset (msb)*/
} IDTGateDesc_t;

/* Our interrupt descriptor table (IDT). */
static IDTGateDesc_t idt[INTERRUPT_MAX];

/* Kernel interrupt functions. */
static InterruptHandler_t int_funcs[INTERRUPT_MAX];

/* Loads the interrupt table. */
static void LoadIDT ();
/* Loads an single entry in the IDT. */
static void LoadIDTEntry (unsigned int interrupt, void (*func)());

/* Gets the interrupt number from the given process. */
static int GetInterruptNumber (Process_t *proc);

/* Enables or disables the IRQ interrupt. */
static void IrqSet (unsigned int irq, int disable);
/* Signals EOI on an interrupt. */
static void IrqEOI (unsigned int irq);

/* Function to be called when an interrupt is unhandled. */
static void UnhandledInterrupt (unsigned int interrupt);

int InterruptInit ()
{
  /* load the IDT */
  LoadIDT();

  /* configure the interrupt controller */
  Config8259s();

  /* load the IDTR */
  IDTPointer_t idtptr;
  idtptr.base = (unsigned int)&idt;
  idtptr.limit = sizeof(idt) - 1;
  __asm__ __volatile__
  ("lidt (%0)" : : "r"(&idtptr));

  return 1;
}

int InterruptDispatch ()
{
  int intnum = GetInterruptNumber(activeproc);
  /* handle the interrupt */
  if (int_funcs[intnum]) {
    int_funcs[intnum](intnum);
  } else {
    UnhandledInterrupt(intnum);
  }
  /* signal EOI */
  if (intnum >= IRQ_BASE && intnum < IRQ_BASE + IRQ_NUM) {
    IrqEOI(intnum - IRQ_BASE);
  }
  return 1;
}

int InterruptHook (unsigned int interrupt, InterruptHandler_t handler)
{
  /* check the int and func are valid */
  if (interrupt >= INTERRUPT_MAX || !handler) {
    return 0;
  }
  /* store the reference */
  int_funcs[interrupt] = handler;
  /* if we are hooking to an IRQ, enable it */
  if (interrupt >= IRQ_BASE && interrupt < IRQ_BASE + IRQ_NUM) {
    IrqSet(interrupt - IRQ_BASE, IRQ_ENABLE);
  }
  return 1;
}

int InterruptUnhook (unsigned int interrupt)
{
  /* check the int is valid and that there is a function to unhook */
  if (interrupt >= INTERRUPT_MAX) {
    return 0;
  }
  if (!int_funcs[interrupt]) {
    return 0;
  }
  /* remove the reference */
  int_funcs[interrupt] = 0;
  /* if we are hooking to an IRQ, disable it */
  if (interrupt >= IRQ_BASE && interrupt < IRQ_BASE + IRQ_NUM) {
    //IrqSet(interrupt - IRQ_BASE, IRQ_DISABLE);
  }
  return 1;
}

static void UnhandledInterrupt (unsigned int interrupt)
{
  kprintf("Unhandled interrupt %d!\n", interrupt);
}

static int GetInterruptNumber (Process_t *proc)
{
  int *stack = (int *)PROC_TO_KERN(proc, proc->stackptr);
  return *(stack + INTERRUPT_NUM_OFFSET);
}

static void Config8259s ()
{
  outb(IO_ICU1, 0x11); /* reset; program device, four bytes */
  BusyWait();
  outb(IO_ICU1+1, IRQ_BASE); /* starting at this vector index */
  BusyWait();
  outb(IO_ICU1+1, IRQ_SLAVE); /* slave on line 2 */
  BusyWait();
  outb(IO_ICU1+1, 1); /* 8086 mode */
  BusyWait();
  outb(IO_ICU1+1, 0xff); /* leave interrupts masked */
  BusyWait();
  outb(IO_ICU1, 0x68); /* special mask mode (if available) */
  BusyWait();
  outb(IO_ICU1, 0x0a); /* read IRR by default */
  BusyWait();

  outb(IO_ICU2, 0x11); /* reset; program device, four bytes */
  BusyWait();
  outb(IO_ICU2+1, IRQ_BASE+8); /* staring at this vector index */
  BusyWait();
  outb(IO_ICU2+1, 2);
  BusyWait();
  outb(IO_ICU2+1, 1); /* 8086 mode */
  BusyWait();
  outb(IO_ICU2+1, 0xff); /* leave interrupts masked */
  BusyWait();
  outb(IO_ICU2, 0x68); /* special mask mode (if available) */
  BusyWait();
  outb(IO_ICU2, 0x0a); /* read IRR by default */
  BusyWait();
}

static void IrqSet (unsigned int irq, int disable)
{
  unsigned int port = IO_ICU1 + 1;
  if (irq >= 8) {
    port = IO_ICU2 + 1;
    irq -= 8;
  }

  unsigned char val = inb(port);
  if (disable) {
    val |= (1 << irq);
  } else {
    val &= ~(1 << irq);
  }
  outb(port, val);
}

static void IrqEOI (unsigned int irq)
{
  outb(IO_ICU1, 0x020);
  outb(IO_ICU2, 0x020);
}

static void BusyWait ()
{
  for (int i = 0; i < 10000; i++) {
    __asm__ __volatile__ ("nop");
  }
}

static void LoadIDTEntry (unsigned int interrupt, void (*func)())
{
  IDTGateDesc_t *ip = &idt[interrupt];
  ip->selector = SEGMENT_KERNEL_CODE;
  ip->unused = 0;
  ip->type = DESCTYPE_INT_INTERRUPT;
  ip->dpl = 0; /* ring 0 */
  ip->present = 1;
  ip->offsetlo = (unsigned int)func;
  ip->offsethi = ((unsigned int)func) >> 16;
}

static void LoadIDT ()
{
  LoadIDTEntry(0, &Interrupt0);
  LoadIDTEntry(1, &Interrupt1);
  LoadIDTEntry(2, &Interrupt2);
  LoadIDTEntry(3, &Interrupt3);
  LoadIDTEntry(4, &Interrupt4);
  LoadIDTEntry(5, &Interrupt5);
  LoadIDTEntry(6, &Interrupt6);
  LoadIDTEntry(7, &Interrupt7);
  LoadIDTEntry(8, &Interrupt8);
  LoadIDTEntry(9, &Interrupt9);
  LoadIDTEntry(10, &Interrupt10);
  LoadIDTEntry(11, &Interrupt11);
  LoadIDTEntry(12, &Interrupt12);
  LoadIDTEntry(13, &Interrupt13);
  LoadIDTEntry(14, &Interrupt14);
  LoadIDTEntry(15, &Interrupt15);
  LoadIDTEntry(16, &Interrupt16);
  LoadIDTEntry(17, &Interrupt17);
  LoadIDTEntry(18, &Interrupt18);
  LoadIDTEntry(19, &Interrupt19);
  LoadIDTEntry(20, &Interrupt20);
  LoadIDTEntry(21, &Interrupt21);
  LoadIDTEntry(22, &Interrupt22);
  LoadIDTEntry(23, &Interrupt23);
  LoadIDTEntry(24, &Interrupt24);
  LoadIDTEntry(25, &Interrupt25);
  LoadIDTEntry(26, &Interrupt26);
  LoadIDTEntry(27, &Interrupt27);
  LoadIDTEntry(28, &Interrupt28);
  LoadIDTEntry(29, &Interrupt29);
  LoadIDTEntry(30, &Interrupt30);
  LoadIDTEntry(31, &Interrupt31);
  LoadIDTEntry(32, &Interrupt32);
  LoadIDTEntry(33, &Interrupt33);
  LoadIDTEntry(34, &Interrupt34);
  LoadIDTEntry(35, &Interrupt35);
  LoadIDTEntry(36, &Interrupt36);
  LoadIDTEntry(37, &Interrupt37);
  LoadIDTEntry(38, &Interrupt38);
  LoadIDTEntry(39, &Interrupt39);
  LoadIDTEntry(40, &Interrupt40);
  LoadIDTEntry(41, &Interrupt41);
  LoadIDTEntry(42, &Interrupt42);
  LoadIDTEntry(43, &Interrupt43);
  LoadIDTEntry(44, &Interrupt44);
  LoadIDTEntry(45, &Interrupt45);
  LoadIDTEntry(46, &Interrupt46);
  LoadIDTEntry(47, &Interrupt47);
  LoadIDTEntry(48, &Interrupt48);
  LoadIDTEntry(49, &Interrupt49);
  LoadIDTEntry(50, &Interrupt50);
  LoadIDTEntry(51, &Interrupt51);
  LoadIDTEntry(52, &Interrupt52);
  LoadIDTEntry(53, &Interrupt53);
  LoadIDTEntry(54, &Interrupt54);
  LoadIDTEntry(55, &Interrupt55);
  LoadIDTEntry(56, &Interrupt56);
  LoadIDTEntry(57, &Interrupt57);
  LoadIDTEntry(58, &Interrupt58);
  LoadIDTEntry(59, &Interrupt59);
  LoadIDTEntry(60, &Interrupt60);
  LoadIDTEntry(61, &Interrupt61);
  LoadIDTEntry(62, &Interrupt62);
  LoadIDTEntry(63, &Interrupt63);
  LoadIDTEntry(64, &Interrupt64);
  LoadIDTEntry(65, &Interrupt65);
  LoadIDTEntry(66, &Interrupt66);
  LoadIDTEntry(67, &Interrupt67);
  LoadIDTEntry(68, &Interrupt68);
  LoadIDTEntry(69, &Interrupt69);
  LoadIDTEntry(70, &Interrupt70);
  LoadIDTEntry(71, &Interrupt71);
  LoadIDTEntry(72, &Interrupt72);
  LoadIDTEntry(73, &Interrupt73);
  LoadIDTEntry(74, &Interrupt74);
  LoadIDTEntry(75, &Interrupt75);
  LoadIDTEntry(76, &Interrupt76);
  LoadIDTEntry(77, &Interrupt77);
  LoadIDTEntry(78, &Interrupt78);
  LoadIDTEntry(79, &Interrupt79);
  LoadIDTEntry(80, &Interrupt80);
  LoadIDTEntry(81, &Interrupt81);
  LoadIDTEntry(82, &Interrupt82);
  LoadIDTEntry(83, &Interrupt83);
  LoadIDTEntry(84, &Interrupt84);
  LoadIDTEntry(85, &Interrupt85);
  LoadIDTEntry(86, &Interrupt86);
  LoadIDTEntry(87, &Interrupt87);
  LoadIDTEntry(88, &Interrupt88);
  LoadIDTEntry(89, &Interrupt89);
  LoadIDTEntry(90, &Interrupt90);
  LoadIDTEntry(91, &Interrupt91);
  LoadIDTEntry(92, &Interrupt92);
  LoadIDTEntry(93, &Interrupt93);
  LoadIDTEntry(94, &Interrupt94);
  LoadIDTEntry(95, &Interrupt95);
  LoadIDTEntry(96, &Interrupt96);
  LoadIDTEntry(97, &Interrupt97);
  LoadIDTEntry(98, &Interrupt98);
  LoadIDTEntry(99, &Interrupt99);
  LoadIDTEntry(100, &Interrupt100);
  LoadIDTEntry(101, &Interrupt101);
  LoadIDTEntry(102, &Interrupt102);
  LoadIDTEntry(103, &Interrupt103);
  LoadIDTEntry(104, &Interrupt104);
  LoadIDTEntry(105, &Interrupt105);
  LoadIDTEntry(106, &Interrupt106);
  LoadIDTEntry(107, &Interrupt107);
  LoadIDTEntry(108, &Interrupt108);
  LoadIDTEntry(109, &Interrupt109);
  LoadIDTEntry(110, &Interrupt110);
  LoadIDTEntry(111, &Interrupt111);
  LoadIDTEntry(112, &Interrupt112);
  LoadIDTEntry(113, &Interrupt113);
  LoadIDTEntry(114, &Interrupt114);
  LoadIDTEntry(115, &Interrupt115);
  LoadIDTEntry(116, &Interrupt116);
  LoadIDTEntry(117, &Interrupt117);
  LoadIDTEntry(118, &Interrupt118);
  LoadIDTEntry(119, &Interrupt119);
  LoadIDTEntry(120, &Interrupt120);
  LoadIDTEntry(121, &Interrupt121);
  LoadIDTEntry(122, &Interrupt122);
  LoadIDTEntry(123, &Interrupt123);
  LoadIDTEntry(124, &Interrupt124);
  LoadIDTEntry(125, &Interrupt125);
  LoadIDTEntry(126, &Interrupt126);
  LoadIDTEntry(127, &Interrupt127);
  LoadIDTEntry(128, &Interrupt128);
  LoadIDTEntry(129, &Interrupt129);
  LoadIDTEntry(130, &Interrupt130);
  LoadIDTEntry(131, &Interrupt131);
  LoadIDTEntry(132, &Interrupt132);
  LoadIDTEntry(133, &Interrupt133);
  LoadIDTEntry(134, &Interrupt134);
  LoadIDTEntry(135, &Interrupt135);
  LoadIDTEntry(136, &Interrupt136);
  LoadIDTEntry(137, &Interrupt137);
  LoadIDTEntry(138, &Interrupt138);
  LoadIDTEntry(139, &Interrupt139);
  LoadIDTEntry(140, &Interrupt140);
  LoadIDTEntry(141, &Interrupt141);
  LoadIDTEntry(142, &Interrupt142);
  LoadIDTEntry(143, &Interrupt143);
  LoadIDTEntry(144, &Interrupt144);
  LoadIDTEntry(145, &Interrupt145);
  LoadIDTEntry(146, &Interrupt146);
  LoadIDTEntry(147, &Interrupt147);
  LoadIDTEntry(148, &Interrupt148);
  LoadIDTEntry(149, &Interrupt149);
  LoadIDTEntry(150, &Interrupt150);
  LoadIDTEntry(151, &Interrupt151);
  LoadIDTEntry(152, &Interrupt152);
  LoadIDTEntry(153, &Interrupt153);
  LoadIDTEntry(154, &Interrupt154);
  LoadIDTEntry(155, &Interrupt155);
  LoadIDTEntry(156, &Interrupt156);
  LoadIDTEntry(157, &Interrupt157);
  LoadIDTEntry(158, &Interrupt158);
  LoadIDTEntry(159, &Interrupt159);
  LoadIDTEntry(160, &Interrupt160);
  LoadIDTEntry(161, &Interrupt161);
  LoadIDTEntry(162, &Interrupt162);
  LoadIDTEntry(163, &Interrupt163);
  LoadIDTEntry(164, &Interrupt164);
  LoadIDTEntry(165, &Interrupt165);
  LoadIDTEntry(166, &Interrupt166);
  LoadIDTEntry(167, &Interrupt167);
  LoadIDTEntry(168, &Interrupt168);
  LoadIDTEntry(169, &Interrupt169);
  LoadIDTEntry(170, &Interrupt170);
  LoadIDTEntry(171, &Interrupt171);
  LoadIDTEntry(172, &Interrupt172);
  LoadIDTEntry(173, &Interrupt173);
  LoadIDTEntry(174, &Interrupt174);
  LoadIDTEntry(175, &Interrupt175);
  LoadIDTEntry(176, &Interrupt176);
  LoadIDTEntry(177, &Interrupt177);
  LoadIDTEntry(178, &Interrupt178);
  LoadIDTEntry(179, &Interrupt179);
  LoadIDTEntry(180, &Interrupt180);
  LoadIDTEntry(181, &Interrupt181);
  LoadIDTEntry(182, &Interrupt182);
  LoadIDTEntry(183, &Interrupt183);
  LoadIDTEntry(184, &Interrupt184);
  LoadIDTEntry(185, &Interrupt185);
  LoadIDTEntry(186, &Interrupt186);
  LoadIDTEntry(187, &Interrupt187);
  LoadIDTEntry(188, &Interrupt188);
  LoadIDTEntry(189, &Interrupt189);
  LoadIDTEntry(190, &Interrupt190);
  LoadIDTEntry(191, &Interrupt191);
  LoadIDTEntry(192, &Interrupt192);
  LoadIDTEntry(193, &Interrupt193);
  LoadIDTEntry(194, &Interrupt194);
  LoadIDTEntry(195, &Interrupt195);
  LoadIDTEntry(196, &Interrupt196);
  LoadIDTEntry(197, &Interrupt197);
  LoadIDTEntry(198, &Interrupt198);
  LoadIDTEntry(199, &Interrupt199);
  LoadIDTEntry(200, &Interrupt200);
  LoadIDTEntry(201, &Interrupt201);
  LoadIDTEntry(202, &Interrupt202);
  LoadIDTEntry(203, &Interrupt203);
  LoadIDTEntry(204, &Interrupt204);
  LoadIDTEntry(205, &Interrupt205);
  LoadIDTEntry(206, &Interrupt206);
  LoadIDTEntry(207, &Interrupt207);
  LoadIDTEntry(208, &Interrupt208);
  LoadIDTEntry(209, &Interrupt209);
  LoadIDTEntry(210, &Interrupt210);
  LoadIDTEntry(211, &Interrupt211);
  LoadIDTEntry(212, &Interrupt212);
  LoadIDTEntry(213, &Interrupt213);
  LoadIDTEntry(214, &Interrupt214);
  LoadIDTEntry(215, &Interrupt215);
  LoadIDTEntry(216, &Interrupt216);
  LoadIDTEntry(217, &Interrupt217);
  LoadIDTEntry(218, &Interrupt218);
  LoadIDTEntry(219, &Interrupt219);
  LoadIDTEntry(220, &Interrupt220);
  LoadIDTEntry(221, &Interrupt221);
  LoadIDTEntry(222, &Interrupt222);
  LoadIDTEntry(223, &Interrupt223);
  LoadIDTEntry(224, &Interrupt224);
  LoadIDTEntry(225, &Interrupt225);
  LoadIDTEntry(226, &Interrupt226);
  LoadIDTEntry(227, &Interrupt227);
  LoadIDTEntry(228, &Interrupt228);
  LoadIDTEntry(229, &Interrupt229);
  LoadIDTEntry(230, &Interrupt230);
  LoadIDTEntry(231, &Interrupt231);
  LoadIDTEntry(232, &Interrupt232);
  LoadIDTEntry(233, &Interrupt233);
  LoadIDTEntry(234, &Interrupt234);
  LoadIDTEntry(235, &Interrupt235);
  LoadIDTEntry(236, &Interrupt236);
  LoadIDTEntry(237, &Interrupt237);
  LoadIDTEntry(238, &Interrupt238);
  LoadIDTEntry(239, &Interrupt239);
  LoadIDTEntry(240, &Interrupt240);
  LoadIDTEntry(241, &Interrupt241);
  LoadIDTEntry(242, &Interrupt242);
  LoadIDTEntry(243, &Interrupt243);
  LoadIDTEntry(244, &Interrupt244);
  LoadIDTEntry(245, &Interrupt245);
  LoadIDTEntry(246, &Interrupt246);
  LoadIDTEntry(247, &Interrupt247);
  LoadIDTEntry(248, &Interrupt248);
  LoadIDTEntry(249, &Interrupt249);
  LoadIDTEntry(250, &Interrupt250);
  LoadIDTEntry(251, &Interrupt251);
  LoadIDTEntry(252, &Interrupt252);
  LoadIDTEntry(253, &Interrupt253);
  LoadIDTEntry(254, &Interrupt254);
  LoadIDTEntry(255, &Interrupt255);
}

