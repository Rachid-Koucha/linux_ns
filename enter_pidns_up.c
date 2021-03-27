/*

  Exemple of child process trying to enter in a pid_ns at
  an upper level

  Author: R. Koucha
  Date: 07-Feb-2020

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


int main(void)
{
pid_t        child;
int          fd, rc;
char         path[256];

  // Build the pathname of the pid_ns
  snprintf(path, sizeof(path), "/proc/%d/ns/pid", getpid());

  // Open the namespace symbolic link
  fd = open(path, O_RDONLY);

  // Create a new pid_ns for child processes
  rc = unshare(CLONE_NEWPID);

  if (rc != 0) {
    ERR("unshare(CLONE_NEWPID): '%m' (%d)\n", errno);
    return 1;
  }

  // Fork a child process
  child = fork();
  if (!child) {

    // Child process

    // Enter into father's pid_ns
    rc = setns(fd, 0);

    if (rc != 0) {
      ERR("setns(father's pid_ns): '%m' (%d)\n", errno);
      exit(1);
    }

    exit(0);

  } else {

    // Father process

    int status;

    // Wait for the end of the child process
    rc = waitpid(child, &status, 0);

    if (rc < 0) {
      ERR("waitpid(%d): %m (%d)", child, errno);
      return 1;
    }

    printf("program's status: %d\n", WEXITSTATUS(status));
  }

  return 0;  
} // main
