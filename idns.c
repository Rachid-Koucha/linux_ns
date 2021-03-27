/*

  Tool displaying the ids of namespaces

  Author: R. Koucha
  Date: 25-Mar-2020

*/


#include <errno.h>
#include <sys/types.h>
#include <stdio.h>
#include <libgen.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "util.h"


int main(int ac, char *av[])
{
  int           i, rc;
  char          tpath[256];
  struct stat   st;
  char         *ns_all[] = {"cgroup", "ipc", "mnt", "net", "pid", "uts", "user", NULL };
  char        **ns;

  if (ac < 2) {
    fprintf(stderr, "Usage: %s pid [ns_names...]\n", basename(av[0]));
    return 1;
  }

  if (!is_pid(av[1])) {
    fprintf(stderr, "Parameter '%s' must be a pid\n", av[1]);
    return 1;
  }

  // If all is passed, use "all"
  if (2 == ac) {
    ns = ns_all;
  } else {
    ns = (char **)0;
  }
  for (i = 2; i < ac; i ++) {
    if (!strcmp(av[i], "all")) {
      ns = ns_all;
      break;
    }
  } // End for

  // Namespaces are on the command line
  if (!ns) {
    ns = &(av[2]);
  }

  // For all the namespaces passed as parameter
  for (i = 0; ns[i]; i ++) {

    if (!is_ns_name(ns[i])) {
      fprintf(stderr, "Unknown namespace name '%s'\n", ns[i]);
      return 1;
    }

    // Build the pathname of the namespace
    snprintf(tpath, sizeof(tpath), "/proc/%s/ns/%s", av[1], ns[i]);

    // Get the information of the namespace
    rc = stat(tpath, &st);
    if (rc < 0) {
      ERR("stat(%s): '%m' (%d)\n", tpath, errno);
      return 1;
    }

    printf("%s [Device,Inode]: [%lu,%lu]\n", tpath, st.st_dev, st.st_ino);

  } // End for

  return 0;

} // main



