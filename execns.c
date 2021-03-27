/*

  Tool running a program in the namespaces of a running process

  Author: R. Koucha
  Date: 22-Jan-2020

*/

#include <errno.h>
#include <sys/types.h>
#include <stdio.h>
#include <libgen.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
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
pid_t        my_pid, tpid, child;
int          n, i, fd, rc;
char        *p, *p1, *p2;
char         tpath[256];

  if (ac < 2) {
    fprintf(stderr, "Usage: %s pid:ns_name1:ns_name2... [program parameters]\n", basename(av[0]));
    return 1;
  }

  // Target pid
  p = strdup(av[1]);
  p1 = p;
  while (*p1) {
    if (*p1 == ':') {
      break;
    }
    p1 ++;
  } // End while

  // If no namespaces on command line, select all of them
  if (*p1 != ':') {
    select_ns("all");
    n = NB_NS_NAME;
  } else {

    *p1 = '\0';
    p1 ++;

    // Get the requested namespaces
    p2 = p1;
    n = 0;
    while (*p2) {
      p2 = strchr(p2, ':');
      if (p2) {
        n += 1;
        *p2 = '\0';
        if (select_ns(p1) != 0) {
          ERR("Bad namespace name '%s'\n", p1);
          return 1;
        }
        p2 ++;
        p1 = p2;
      } else {

        // Last name
        n += 1;
        if (select_ns(p1) != 0) {
          ERR("Bad namespace name '%s'\n", p1);
          return 1;
        }
        break;
      }
    } // End while

    if (!n) {
      select_ns("all");
      n = NB_NS_NAME;
    }
  } // End if no namespace specified

  // Target pid
  if (!is_pid(p)) {
    fprintf(stderr, "Parameter '%s' must be a pid\n", p);
    return 1;
  }
  tpid = atoi(p);
  free(p);

  // My pid
  my_pid = getpid();

  // For each namespace of the target process:
  //   . Build the pathname to the symbolic link
  //   . Get the identifier and compare it to the calling process one
  //   . If different, enter into the namespace
  for (i = 0; i < NB_NS_NAME; i ++) {

    // If this namespace is requested
    if (ns_list[i].selected) {

      // Compare the identifiers
      if (cmp_ns(my_pid, tpid, ns_list[i].name)) {
        // Don't do anything if same namespace
        continue;
      }

      // Build the pathname
      snprintf(tpath, sizeof(tpath), "/proc/%d/ns/%s", tpid, ns_list[i].name);

      // Open the target namespace symbolic link
      fd = open(tpath, O_RDONLY);
      if (fd < 0) {
        ERR("open(%s): '%m (%d)\n", tpath, errno);
        return 1;
      }

      // Enter into the target namespace
      rc = setns(fd, ns_list[i].flag);

      if (rc != 0) {
        ERR("Unable to enter into namespace '%s' (%m-%d)\n", ns_list[i].name, errno);
        return 1;
      } else {
        printf("Moved into namespace %s\n", ns_list[i].name);
      }

      close(fd);
    } // End if namespace selected
  } // End for

  // Fork a child process
  child = fork();
  if (!child) {

    // Child process

    char *cmd;

    // If no command is passed
    if (ac < 3) {
      char *av_cmd[] = { DEFAULT_CMD, NULL };

      cmd = av_cmd[0];
      execv(cmd, av_cmd);

    } else {

      cmd = av[2];
      execv(cmd, &(av[2]));

    }

    ERR("Unable to run '%s': %m (%d)\n", cmd, errno);
    _exit(1);

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
