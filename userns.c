/*

Tool which determine the user/uid of a user_ns

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
#include <fcntl.h>
#include <linux/nsfs.h>
#include <string.h>
#include <pwd.h>

#include "util.h"


int main(int ac, char *av[])
{
  int            tfd, rc;
  char           tpath[256];
  uid_t          uid;
  struct passwd *pw;

  if (ac != 2) {
    fprintf(stderr, "Usage: %s pid\n", basename(av[0]));
    return 1;
  }

  // Check pid
  if (!is_pid(av[1])) {
    fprintf(stderr, "Parameter '%s' must be a pid\n", av[1]);
    return 1;
  }

  // Build the pathname of the namespace
  snprintf(tpath, sizeof(tpath), "/proc/%s/ns/user", av[1]);

  // Open the pathname of the namespace
  tfd = open(tpath, O_RDONLY);
  if (tfd < 0) {
    ERR("open(%s): '%m' (%d)\n", tpath, errno);
    return 1;
  }

  // Get the owner's uid to which this namespace belongs to
  rc = ioctl(tfd, NS_GET_OWNER_UID, &uid);

  if (rc < 0) {
    ERR("ioctl(%s, NS_GET_OWNER_UID): '%m' (%d)\n", tpath, errno);
    return 1;
  }

  // Get the owner's password entry to get his name
  pw = getpwuid(uid);

  if (!pw) {
    ERR("getpwuid(%d): '%m' (%d)\n", uid, errno);
    return 1;
  }

  printf("%s belongs to user: '%s' (%d)\n", tpath, pw->pw_name, uid);

  close(tfd);

  return 0;

} // main



