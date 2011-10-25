/*
 * System call error messages.
 */
#ifndef __ERRMSG_H
#define __ERRMSG_H

/* Error messages. */
const char *const errmsgs[] = 
{
  "Operation succeeded",
  "Unknown failure",
  "No such system call",
  "Invalid argument",
  "Insufficient memory",
  "Event has waiting process",
  "Message is too big",
  "Sender is invalid",
  "Reply not expected",
  "Name is too long"
};

/* Number of error messages. */
const int nerr = sizeof(errmsgs) / sizeof (const char *);

#endif

