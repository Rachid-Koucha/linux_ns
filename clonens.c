/*

  Tool running a child process in new namespaces

  Author: R. Koucha
  Date: 22-Jan-2020

*/

#include <errno.h>
#include <stdio.h>
#include <libgen.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

#include "util.h"


#define CHILD_STACK_SZ (500 * 1024)

static char child_stack[CHILD_STACK_SZ];


#define NB_NS_NAME 7

struct ns_type_t
{
  char *name;
  int   flag;
} ns[NB_NS_NAME] = {
  { "cgroup", CLONE_NEWCGROUP },
  { "ipc",    CLONE_NEWIPC    },
  { "mnt",    CLONE_NEWNS     },
  { "net",    CLONE_NEWNET    },
  { "pid",    CLONE_NEWPID    },
  { "user",   CLONE_NEWUSER   },
  { "uts",    CLONE_NEWUTS    }
};

int ns_name2type(const char *name)
{
  int i;

  for (i = 0; i < NB_NS_NAME; i ++) {
    if (!strcmp(ns[i].name, name)) {
      return ns[i].flag;
    }
  }

  return -1;

} // ns_name2type



static int child(void *p)
{
  (void)p;

  pause();

  return 0;

} // child



int main(int ac, char *av[])
{
int         i, rc, flags;
pid_t       cloneid;

  if (ac < 2) {
    fprintf(stderr, "Usage: %s ns_name1 ns_name2...\n", basename(av[0]));
    return 1;
  }

  // For all the namespaces passed as parameter, determine the
  // clone flag
  flags = 0;
  for (i = 1; i < ac; i ++) {
    // Translate the name into type
    rc = ns_name2type(av[i]);
    if (rc < 0) {
      ERR("Bad namespace name '%s'\n", av[i]);
      return 1;
    }

    // Add the clone flag to the mask
    flags |= rc;
  } // End for

  // Clone a process
  cloneid = clone(child, child_stack + CHILD_STACK_SZ, SIGCHLD | flags, 0);

  if (cloneid < 0) {
    ERR("clone(0x%x): '%m' (%d)\n", flags, errno);
    return 1;
  }

  printf("Created process %d in requested namespaces\n", cloneid);

  // Wait for the end of the process
  waitpid(cloneid, 0, 0);

  return 0;
} // main
