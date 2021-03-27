/*

  Tool which determine the user_ns owning the namespaces of a given process

  Author: R. Koucha
  Date: 25-Mar-2020

*/


#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <libgen.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/nsfs.h>
#include <string.h>

#include "util.h"


int main(int ac, char *av[])
{
  int           tfd, ufd, i;
  char          tpath[256];
  struct stat   st;
  char         *ns_all[] = {"cgroup", "ipc", "mnt", "net", "pid", "uts", "user", NULL };
  char        **ns;
  int           rc;

  if (ac < 2) {
    fprintf(stderr, "Usage: %s pid [ns_names...]\n", basename(av[0]));
    return 1;
  }

  // Check pid
  if (!is_pid(av[1])) {
    fprintf(stderr, "First parameter '%s' must be a pid\n", av[1]);
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
  for (rc = 0, i = 0; ns[i]; i ++) {

    if (!is_ns_name(ns[i])) {
      fprintf(stderr, "Unknown namespace name '%s'\n", ns[i]);
      return 1;
    }

    // Build the pathname of the namespace
    snprintf(tpath, sizeof(tpath), "/proc/%s/ns/%s", av[1], ns[i]);

    // Open the pathname of the namespace
    tfd = open(tpath, O_RDONLY);
    if (tfd < 0) {
      ERR("open(%s): '%m' (%d)\n", tpath, errno);
      return 1;
    }

    // Get a file descriptor on the user_ns to which this namespace
    // belongs to
    ufd = ioctl(tfd, NS_GET_USERNS);

    if (ufd < 0) {
      ERR("ioctl(%s, NS_GET_USERNS): '%m' (%d)\n", tpath, errno);
      // Continue with other namespaces
      rc = 1;
    } else {

      fstat(ufd, &st);

      printf("%s belongs to [Device,Inode]: [%lu,%lu]\n", tpath, st.st_dev, st.st_ino);
    }

    close(tfd);
    close(ufd);
  } // End for

  return rc;

} // main



