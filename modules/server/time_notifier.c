/*
 * Time server notifier.
 */
#include <stdio.h>
#include <string.h>

#include <timeserver.h>
#include <syscall.h>
#include <irq.h>

#include <cs452/machine/cpufunc.h>
#include <cs452/machine/pio.h>

#define	IO_TIMER1	(0x040) /* 8253 timer 1 */
#define	TIMER_CNTR0 (IO_TIMER1 + 0) /* timer counter port */
#define	TIMER_MODE (IO_TIMER1 + 3) /* timer mode port */

#define	TIMER_SEL0 (0x00) /* select counter 0 */
#define	TIMER_RATEGEN (0x04) /* mode 2, rate generator */
#define	TIMER_16BIT (0x30) /* r/w counter 16 bits, LSB first */

#define	TIMER_FREQ (1193182)
#define TIMER_DIV(x) ((TIMER_FREQ+(x)/2)/(x))

#define MESSENGER_POOL_SIZE (4)

/* Our courier. */
static int courier_pid;

/* Initialize and run the notifier. */
static void Init();
static void Run();

int main ()
{
  Init();
  Run();
  Exit();
}

static void Init ()
{
  TimeNotifierCfg_t init;

  int server_pid = Receive((char *)&init, sizeof(init));
  if (server_pid <= 0) {
    printf("time notifier: init Receive failed (%s)\n", strerror(server_pid));
    Exit();
  }
  courier_pid = init.courier;

  outb(TIMER_MODE, TIMER_SEL0 | TIMER_RATEGEN | TIMER_16BIT);
  outb(TIMER_CNTR0, TIMER_DIV(init.hz) % 256);
  outb(TIMER_CNTR0, TIMER_DIV(init.hz) / 256);

  int result = Reply(server_pid, 0, 0);
  if (result < 0) {
    printf("time notifier: init Reply failed (%s)\n", strerror(result));
    Exit();
  }
}

static void Run ()
{
  int result;
  for (;;) {
    result = AwaitEvent(IRQ_TIMER);
    if (result < 0) {
      printf("time notifier: AwaitEvent failed (%s)\n", strerror(result));
      Exit();
    }
    result = Send(courier_pid, 0, 0, 0, 0);
    if (result < 0) {
      printf("time notifier: Send failed (%s)\n", strerror(result));
      Exit();
    }
  }
}

