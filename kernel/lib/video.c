#include "video.h"

#define VIDEO_MEMORY (VGA_LINEAR_LOCATION + VGA_TEXT_OFFSET)
#define VIDEO_WIDTH 80
#define VIDEO_HEIGHT 25

/* The character attribute to apply. */
static int video_attr = 7;

/* The current cursor line. */
static unsigned int currentline;
/* The current cursor column. */
static unsigned int currentcolumn;

/* Places a character at the given xy coordinates. */
static void VideoXYPut (int x, int y, char c);

/* Clears a line of text. */
static void VideoClearLine (int y);

int VideoInit ()
{
  /* clear the screen */
  for (int i = 0; i < VIDEO_WIDTH * VIDEO_HEIGHT; i++) {
    VideoXYPut(i % VIDEO_WIDTH, i / VIDEO_WIDTH, ' ');
  }
  /* reset the cursor position */
  currentline = 0;
  currentcolumn = 0;
  return 1;
}

char VideoGetAttr ()
{
  return video_attr;
}

void VideoSetAttr (char attr)
{
  video_attr = attr;
}

void VideoPut (char out)
{
  if (out == '\n') {
    currentcolumn = 0;
    currentline = (currentline + 1) % VIDEO_HEIGHT;
    VideoClearLine(currentline);
  } else {
    VideoXYPut(currentcolumn, currentline, out);
    currentcolumn += 1;
  }
}

int VideoWrite (char *out)
{
  int chars = 0;
  for (chars = 0; *out; out++, chars++) {
    VideoPut(*out);
  }
  return chars;
}

static void VideoXYPut (int x, int y, char c)
{
  char *memory = (char*)(VIDEO_MEMORY + 2 * (VIDEO_WIDTH * y + x));
  *memory = c;
  *(memory + 1) = video_attr;
}

static void VideoClearLine (int y)
{
  for (int x = 0; x < VIDEO_WIDTH; x++) {
    VideoXYPut(x, y, ' ');
  }
}

