/*

  Tool setting the hostname in an uts_namespace

  Author: R. Koucha
  Date: 31-Jan-2020

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

#include "util.h"


int main(int ac, char *av[])
{
int          fd, rc;
char         tpath[256];

  if (ac != 3) {
    fprintf(stderr, "Usage: %s pid hostname\n", basename(av[0]));
    return 1;
  }

  if (!is_pid(av[1])) {
    fprintf(stderr, "First parameter '%s' must be a pid\n", av[1]);
    return 1;
  }

  // Build the pathname of the uts_ns
  snprintf(tpath, sizeof(tpath), "/proc/%s/ns/uts", av[1]);

  // Open the target uts_ns symbolic link
  fd = open(tpath, O_RDONLY);

  // Enter into the target namespace
  rc = setns(fd, CLONE_NEWUTS);

  if (rc != 0) {
    ERR("Unable to enter into network_ns\n");
    return 1;
  }

  close(fd);

  rc = sethostname(av[2], strlen(av[2]));
  if (rc != 0) {
    ERR("sethostname(%s): %m (%d)\n", av[2], errno);
    return 1;
  }

  return 0;  
} // main
