/*
 * Commands for reading from and writing to the terminal.
 */
#ifndef __TERMINAL_H
#define __TERMINAL_H

/* The terminal size. */
#define TERMINAL_WIDTH (80)
#define TERMINAL_HEIGHT (25)

/* The terminal colours. */
#define TERMINAL_FG_WHITE (37)
#define TERMINAL_FG_GREEN (32)
#define TERMINAL_FG_RED (31)

#define TERMINAL_BG_BLACK (40)
#define TERMINAL_BG_WHITE (47)
#define TERMINAL_BG_BLUE (44)

/* The maximum number of characters in a line. */
#define TERMINAL_LINE_MAX (40)

/*
 * Initializes the terminal serial port.
 */
void TerminalInit ();

/*
 * Clears the terminal and places the cursor in the upper left.
 */
void TerminalClear ();

/*
 * Prints the data to the terminal starting at the current cursor position.
 */
int TerminalPrintf (const char *format, ...);

/*
 * Prints data to the terminal at the given position.
 */
int TerminalRCPrintf (unsigned int r, unsigned int c, const char *format, ...);

/*
 * Reads a single character from the terminal.
 */
char TerminalRead ();

/*
 * Reads a full line from the terminal, echoing it at the given location.
 */
char *TerminalReadLine (int echox, int echoy);

/*
 * Sets the foreground and background colours.
 */
void TerminalSetColours (int fg, int bg);

#endif

