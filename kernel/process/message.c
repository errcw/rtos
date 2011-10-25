#include "message.h"
#include "schedule.h"
#include "abscopy.h"
#include "schedule.h"
#include "ksyscall.h"
#include "kern/limits.h"
#include "kern/error.h"

/* A single message. */
typedef struct MessageInfo {
  char *message;
  int message_len;
  Pid_t pid;
} MessageInfo_t;

/* An entry in the send queue. */
typedef struct SendQueue {
  MessageInfo_t msgs[PROCESS_MAX];
  unsigned int head;
  unsigned int tail;
  unsigned int size;
} SendQueue_t;

/* The send queue for each process. Indexed by receiver. */
static SendQueue_t sendq[PROCESS_MAX];
/* The pending receives. Indexed by receiver. */
static MessageInfo_t receives[PROCESS_MAX];
/* The pending replies. Indexed by sender. */
static MessageInfo_t replies[PROCESS_MAX];

/* Sends the data from one process to another. */
static int Transfer (MessageInfo_t *rcv, MessageInfo_t *snd);


int MessageInit ()
{
 for (int mes = 0; mes < PROCESS_MAX; mes++) {
    sendq[mes].head = 0;
    sendq[mes].tail = 0;
    sendq[mes].size = 0;
  }
  return 1;
}

int MessageSend (Process_t *recv, Process_t *send, char *msg, int msglen, char *rpl, int rpllen)
{
  /* note the send and reply info */
  int recv_pidx = PID_INDEX(recv->pid);
  MessageInfo_t send_info;
  send_info.message = msg;
  send_info.message_len = msglen;
  send_info.pid = send->pid;

  int send_pidx = PID_INDEX(send->pid);
  MessageInfo_t *recv_info = &replies[send_pidx];
  recv_info->message = rpl;
  recv_info->message_len = rpllen;
  recv_info->pid = recv->pid;

  if (recv->state == BLOCKED_RECEIVE) {
    /* already waiting for a send: copy the data */
    int err = Transfer(&receives[recv_pidx], &send_info);
    if (!err) {
      send->state = BLOCKED_REPLY; /* wait for a reply */
      SyscallSetReturn(recv, send->pid); /* note who sent the messge */
    } else {
      SyscallSetReturn(recv, err); /* fail the waiting receiver */
    }
    /* unblock the receiver */
    ScheduleAdd(recv);
    
    return err;

  } else {
    /* wait on the send queue */
    sendq[recv_pidx].msgs[ sendq[recv_pidx].tail ] = send_info;
    sendq[recv_pidx].tail = (sendq[recv_pidx].tail + 1) % PROCESS_MAX;
    sendq[recv_pidx].size = (sendq[recv_pidx].size + 1);
    /* block the sending process */
    send->state = BLOCKED_SEND;

    return EOK;
  }
}

int MessageReceive (Process_t *recv, char *msg, int msglen)
{
  int recv_pidx = PID_INDEX(recv->pid);

  /* note the receive info */
  MessageInfo_t *recv_info = &receives[recv_pidx];
  recv_info->message = msg;
  recv_info->message_len = msglen;
  recv_info->pid = recv->pid;

  if (sendq[recv_pidx].size > 0) {
    /* we have senders waiting: grab the start of the queue */
    MessageInfo_t *send_info = &sendq[recv_pidx].msgs[ sendq[recv_pidx].head ];
    sendq[recv_pidx].head = (sendq[recv_pidx].head + 1) % PROCESS_MAX;
    sendq[recv_pidx].size = (sendq[recv_pidx].size - 1);

    /* ensure the sending process is still valid */
    Process_t *send = ProcessFind(send_info->pid);
    if (!send || send->state != BLOCKED_SEND) {
      return ENOSEND;
    }

    int err = Transfer(recv_info, send_info);
    if (!err) {
      send->state = BLOCKED_REPLY; /* block the sender for a reply */
      return send_info->pid; /* note who sent the message */
    } else {
      SyscallSetReturn(send, err); /* fail and unblock the sender */
      ScheduleAdd(send);
      return err; /* fail the receiver */
    }

  } else {
    /* block and wait for data */
    recv->state = BLOCKED_RECEIVE;

    return EOK;
  }
}

int MessageReply (Process_t *send, Process_t *reply, char *rpl, int rpllen)
{
  /* make sure the sender is waiting for a reply */
  if (send->state != BLOCKED_REPLY) {
    return ENOSEND;
  }
  /* make sure it wants a reply from the given source */
  int send_pidx = PID_INDEX(send->pid);
  if (replies[send_pidx].pid != reply->pid) {
    return ENOREPL;
  }

  MessageInfo_t reply_info;
  reply_info.message = rpl;
  reply_info.message_len = rpllen;

  int err = Transfer(&replies[send_pidx], &reply_info);
  if (err) {
    SyscallSetReturn(send, err); /* fail the original sender */
  }

  /* unblock the original sender */
  ScheduleAdd(send);

  /* return whether the reply succeeded */
  return err;
}

void MessageNotifyDestroyed (Process_t *proc)
{
  int pidx = PID_INDEX(proc->pid);

  /* wake up any processes waiting on the send queue */
  while (sendq[pidx].size > 0) {
    Pid_t spid = sendq[pidx].msgs[ sendq[pidx].head ].pid;
    sendq[pidx].head = (sendq[pidx].head + 1) % PROCESS_MAX;
    sendq[pidx].size = (sendq[pidx].size - 1);

    Process_t *sp = ProcessFind(spid);
    if (sp) {
      SyscallSetReturn(sp, 0);
      ScheduleAdd(sp);
    }
  }

  /* wake up any processes waiting for replies */
  for (int p = 0; p < PROCESS_MAX; p++) {
    if (replies[p].pid == proc->pid) {
      Process_t *sp = ProcessFind(replies[p].pid);
      if (sp && sp->state == BLOCKED_REPLY) {
        SyscallSetReturn(sp, 0);
        ScheduleAdd(sp);
      }
    }
  }

}

static int Transfer (MessageInfo_t *recv_info, MessageInfo_t *send_info)
{
  /* check that the receiver can receive the full message */
  if (send_info->message_len > recv_info->message_len) {
    return EM2BIG;
  }

  /* transfer the data */
  if (send_info->message_len > 0) {
    AbsCopy((unsigned int)recv_info->message,
            (unsigned int)send_info->message,
            send_info->message_len);
  }

  return EOK;
}

