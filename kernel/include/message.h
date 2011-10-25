/*
 * Implements message passing using Send/Receive/Reply.
 */
#ifndef __MESSAGE_H
#define __MESSAGE_H

#include "process.h"

/*
 * Initialses the message passing subsystem.
 */
int MessageInit ();

/*
 * Initiates a message transfer from one process to another. If no  process is
 * available to receive the message the sending process blocks.
 */
int MessageSend (Process_t *recv, Process_t *send, char *msg, int msglen, char *rpl, int rpllen);

/*
 * Receives a message for the given process into the specified buffer. If no message
 * is immediately available, the receiving process blocks.
 */
int MessageReceive (Process_t *recv, char *msg, int msglen);

/*
 * Sends a reply from the active process.
 */
int MessageReply (Process_t *send, Process_t *reply, char *rpl, int rpllen);

/*
 * Notifies the message passing system that the given process is being
 * destroyed so that processes blocked on its send queue can be woken.
 */
void MessageNotifyDestroyed (Process_t *proc);

#endif

