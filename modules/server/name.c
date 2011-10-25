#include <string.h>
#include <stdio.h>

#include "nameserver.h"
#include "syscall.h"
#include "limits.h"

/* The size of the receive buffer. */
#define MESSAGE_BUFFER_SIZE (NAME_MAXLEN + 1)

/* A prime number for use in the hash algorithm. */
#define HASH_PRIME (100711433)

/* A bucket in our coalesced hash map. */
typedef struct Bucket {
  char name[NAME_MAXLEN];
  int pid;

  int next; /* the next bucket in the chain */
} Bucket_t;

/* The map of names to pids. */
static Bucket_t namemap[NAME_MAX];

/* Initializes the name server. */
static void Init ();
/* Receives and replies to name requests. */
static void Run ();

/* Registers a pid with the given name. */
static int Register (int pid, char *name);
/* Looks up a name and returns the associated pid, or zero for none. */
static int Lookup (char *name);

/* Calculates a hash code for the name. */
static unsigned int Hash (char *name);


int main ()
{
  Init();
  Run();
  Exit();
}

static void Init ()
{
  for (int i = 0; i < NAME_MAX; i++) {
    namemap[i].pid = 0;
    namemap[i].name[0] = '\0';
    namemap[i].next = -1;
  }
  RegisterAsNameserver();
}

static void Run ()
{
  char msgbuf[MESSAGE_BUFFER_SIZE];

  for (;;) {
    /* receive a request */
    int pid = Receive(msgbuf, MESSAGE_BUFFER_SIZE);
    if (pid) {
      char type = msgbuf[0];
      char *name = &msgbuf[1];

      int response;
      if (type == NAMESERVER_WHOIS) {
        /* look up a name */
        response = Lookup(name);
      } else if (type == NAMESERVER_REGISTER) {
        /* register a name */
        response = Register(pid, name);
      }

      Reply(pid, (char *)&response, sizeof(response));
    }
  }
}

static int Register (int pid, char *name)
{
  unsigned int h = Hash(name) % NAME_MAX;

  if (!namemap[h].pid) {
    /* start a new chain */
    namemap[h].pid = pid;
    strcpy(namemap[h].name, name);

  } else {
    int bkt = 0, ch;

    /* search along the existing chain */
    for (ch = h; namemap[ch].next < 0; ch = namemap[ch].next) {
      /* replace an existing entry */
      if (strcmp(name, namemap[ch].name) == 0) {
        namemap[ch].pid = pid;
        return 1;
      }
    }

    /* find the first empty bucket */
    while (bkt < NAME_MAX && namemap[bkt].pid) {
      bkt++;
    }
    /* abort if the map is full */
    if (bkt == NAME_MAX) {
      return 0;
    }

    /* insert the values */
    namemap[bkt].pid = pid;
    strcpy(namemap[bkt].name, name);

    /* point to the new last node in the chain */
    namemap[ch].next = bkt;
  }

  return 1;
}

static int Lookup (char *name)
{
  unsigned int h = Hash(name) % NAME_MAX;

  /* check for a chain */
  if (namemap[h].pid) {
    int ch;
    /* search along the chain */
    for (ch = h; namemap[ch].pid; ch = namemap[ch].next) {
      if (strcmp(name, namemap[ch].name) == 0) {
        return namemap[ch].pid;
      }
    }
  }

  /* no matching name in the map */
  return 0;
}

static unsigned int Hash (char *name)
{
  unsigned int hash;
  unsigned int len = strlen(name);

  /* use Knuth's ACP hash algorithm */
  for (hash = len; len; len--) {
    hash = ((hash << 5) ^ (hash >> 27)) ^ *name++;
  }
  hash = hash % HASH_PRIME;

  return hash;
}

