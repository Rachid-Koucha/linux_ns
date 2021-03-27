/*
   Utilities for the examples

  Author: R. Koucha
  Date: 25-Mar-2020

*/

#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#include "util.h"



int is_pid(const char *str)
{
const char *p = str;

  while (*p) {
    if (!isdigit(*p)) {
      return 0;
    }
    p ++;
  } // End while

  return 1;
} // is_pid


int is_integer(const char *str)
{
const char *p = str;

  if ('-' == *p) {
    p ++;
  }

  return is_pid(p);
} // is_integer



int is_ns_name(const char *ns)
{
const char *ns_all[] = {"cgroup", "ipc", "mnt", "net", "pid", "uts", "user", NULL };
int         i;

  for (i = 0; ns_all[i]; i ++) {
    if (!strcmp(ns, ns_all[i])) {
      return 1;
    }
  } // End for

  return 0;
} // is_valid_ns


int getanswer(void)
{
  int c, c1;

  c = fgetc(stdin);

  // Flush stdin
  c1 = c;
  while (c1 != '\n') {
    c1 = fgetc(stdin);
  }

  return c;
} // getanswer


int cmp_ns(pid_t pid1, pid_t pid2, const char *ns_name)
{
char path1[256], path2[256];
struct stat stbuf1, stbuf2;
int rc;

  rc = snprintf(path1, sizeof(path1), "/proc/%d/ns/%s", pid1, ns_name);
  if (rc < 0 || rc >= (int)sizeof(path1)) {
    if (rc >= 0) {
      errno = ENOSPC;
    }
    return -1;
  }
  rc = snprintf(path2, sizeof(path2), "/proc/%d/ns/%s", pid2, ns_name);
  if (rc < 0 || rc >= (int)sizeof(path2)) {
    if (rc >= 0) {
      errno = ENOSPC;
    }
    return -1;
  }

  rc = stat(path1, &stbuf1);
  if (rc < 0) {
    ERR("stat(%s): '%m' (%d)\n", path1, errno);
    return -1;
  }
  rc = stat(path2, &stbuf2);
  if (rc < 0) {
    ERR("stat(%s): '%m' (%d)\n", path2, errno);
    return -1;
  }

  // Comparison of the identifiers
  if ((stbuf1.st_dev == stbuf2.st_dev) &&
      (stbuf1.st_ino == stbuf2.st_ino)) {
    return 1;
  } else {
    return 0;
  }
 
} // cmp_ns



