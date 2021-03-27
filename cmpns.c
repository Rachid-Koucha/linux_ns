/*

  Simple program to compare the namespaces of two processes

  Author: R. Koucha
  Date: 25-Mar-2020

*/


#include <errno.h>
#include <stdio.h>
#include <libgen.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "util.h"


#define NB_NS_NAME 7
char *ns_name[NB_NS_NAME] = {
  "cgroup",
  "ipc",
  "mnt",
  "net",
  "pid",
  "user",
  "uts"
};


int main(int ac, char *av[])
{
struct stat stbuf1, stbuf2;
char        path1[256], path2[256];
int         i;
int         rc;

  if (ac != 3) {
    fprintf(stderr, "Usage: %s pid1 pid2\n", basename(av[0]));
    return 1;
  }

  if (!is_pid(av[1])) {
    fprintf(stderr, "First parameter '%s' must be a pid\n", av[1]);
    return 1;
  }

  if (!is_pid(av[2])) {
    fprintf(stderr, "Second parameter '%s' must be a pid\n", av[2]);
    return 1;
  }

  // Return 0, if the namespaces are equal
  // Return 1, otherwise
  rc = 0;

  // For all the namespaces
  for (i = 0; i < NB_NS_NAME; i ++) {

    // Build the pathnames of the namespaces
    snprintf(path1, sizeof(path1), "/proc/%s/ns/%s", av[1], ns_name[i]);
    snprintf(path2, sizeof(path2), "/proc/%s/ns/%s", av[2], ns_name[i]);

    rc = stat(path1, &stbuf1);
    if (rc != 0) {
      ERR("stat(%s): '%m' (%d)\n", path1, errno);
      // Continue with next namespace
      continue;
    }
    rc = stat(path2, &stbuf2);
    if (rc != 0) {
      ERR("stat(%s): '%m' (%d)\n", path2, errno);
      // Continue with next namespace
      continue;
    }

    // Comparison of the identifiers
    if ((stbuf1.st_dev == stbuf2.st_dev) &&
	(stbuf1.st_ino == stbuf2.st_ino)) {
      printf("%s is equal\n", ns_name[i]);
    } else {
      printf("%s is different\n", ns_name[i]);
      rc = 1;
    }
  } // End for

  return rc;
} // main
