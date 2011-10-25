#ifndef __VIDEO_H
#define __VIDEO_H

#define VGA_LINEAR_LOCATION (0x000A0000)
#define VGA_MEMORY_SIZE (0x20000)
#define VGA_TEXT_OFFSET 0x00018000
#define SVGA_LINEAR_LOCATION (0x80000000)

/*
 * Initializes video memory. Clears the screen.
 */
int VideoInit ();

char VideoGetAttr ();
void VideoSetAttr (char attr);

void VideoPut (char out);
int VideoWrite (char *out);

#endif

