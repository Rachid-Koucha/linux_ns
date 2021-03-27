/*

  Tool running a shell in new namespaces with a user prompt

  Author: R. Koucha
  Date: 15-Fev-2020

*/

#include <errno.h>
#include <sys/types.h>
#include <stdio.h>
#include <libgen.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

#include "util.h"

#define DEFAULT_CMD   "/bin/sh"








#define NB_NS_NAME 7

struct ns_list_t {
  char *name;
  int   selected;
  int   flag;
} ns_list[NB_NS_NAME] = {
  { "ipc",    0, CLONE_NEWIPC    },
  { "pid",    0, CLONE_NEWPID    },
  { "net",    0, CLONE_NEWNET    },
  { "user",   0, CLONE_NEWUSER   },
  { "uts",    0, CLONE_NEWUTS    },
  { "cgroup", 0, CLONE_NEWCGROUP },
  { "mnt",    0, CLONE_NEWNS     }
};

// Set the "selected" field in ns_list[] for the given
// namespace or all (if keyword "all" is passed)
static int select_ns(const char *ns)
{
  int i, fnd;

  fnd = 0;
  for (i = 0; i < NB_NS_NAME; i ++) {
    if (!strcmp("all", ns)) {
      ns_list[i].selected = 1;
      fnd = 1;
    } else {
      if (!strcmp(ns_list[i].name, ns)) {
        ns_list[i].selected = 1;
        return 0;
      }
    }
  } // End for

  // Bad namespace name
  if (!fnd) {
    return -1;
  }

  return 0;

} // select_ns


int main(int ac, char *av[])
{
pid_t        child;
int          i, rc;
int          flags;

  if ((ac > 1) && !strcmp(av[1], "-h")) {
    fprintf(stderr, "Usage: %s [ns_name1 ns_name2... | all]\n", basename(av[0]));
    return 0;
  }

  // If no namespaces on command line, select all of them
  if (ac == 1) {
    select_ns("all");
  } else {

    for (i = 1; i < ac; i ++) {
      // Get the requested namespaces
      rc = select_ns(av[i]);
      if (rc != 0) {
        ERR("Bad namespace '%s'\n", av[i]);
        return 1;
      }
    } // End for
  } // End if no namespace specified

  // For each namespace of the target process, set the flag
  flags = 0;
  for (i = 0; i < NB_NS_NAME; i ++) {
    if (ns_list[i].selected) {
      printf("New namespace '%s'\n", ns_list[i].name);
      flags |= ns_list[i].flag;
    } // End if namespace selected
  } // End for

  // Create brand new namespaces
  rc = unshare(flags);
  if (rc != 0) {
    ERR("unshare(0x%x): '%m' (%d)\n", flags, errno);
    return 1;
  }

  // Fork a child process
  child = fork();
  if (!child) {

    // Child process

    int   c;
    char *av_cmd[] = { DEFAULT_CMD, NULL };

    prompt("Process#%d go forward ([Y]/N)? ", getpid());
    c = getanswer();
    if ('\n' == c || 'y' == c || 'Y' == c) {
      execv(av_cmd[0], av_cmd);

      ERR("Unable to run '%s': %m (%d)\n", av_cmd[0], errno);
      _exit(1);
    }

    return 1;

  } else {

    // Father process

    int status;

    // Wait for the end of the child process
    rc = waitpid(child, &status, 0);

    if (rc < 0) {
      ERR("waitpid(%d): %m (%d)", child, errno);
      return 1;
    }

    printf("program's status: %d (0x%x)\n", status, status);
  }

  return 0;  
} // main
