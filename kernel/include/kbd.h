#ifndef __KBD_H
#define __KBD_H

/* IBM keyboard input */
void console_kbdreset();            /* call first */
int  console_kbhit();               /* return T if char waiting */
char console_getchar();             /* returns ASCII for keys as pressed */

#endif

