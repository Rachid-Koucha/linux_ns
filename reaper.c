/*

  Tool reaping the dead processes in a pid namespaces

  Author: R. Koucha
  Date: 11-Apr-2020

*/


#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>


#include "util.h"



static const char *signame[] = {
    "",
    "HUP",
    "INT",
    "QUIT",
    "ILL",
    "TRAP",
    "ABRT",
    "BUS",
    "FPE",
    "KILL",
    "USR1",
    "SEGV",
    "USR2",
    "PIPE",
    "ALRM",
    "TERM",
    "STKFLT",
    "CHLD",
    "CONT",
    "STOP",
    "TSTP",
#define MAX_SIG SIGTSTP
    NULL
};


static int sig_str2type(char *s)
{
  int i = 0;

  for (i = 0; signame[i]; i ++) {
    if (!strcmp(signame[i], s)) {
      return i;
    }
  } // End for

  return -1;
} // sig_str2type



#define REAPER_NAME "Reaper"


static pid_t pid_sh;


static void sig_hdl(
                    int        sig,
                    siginfo_t *info,
                    void      *p
                   )
{
pid_t pid;
int   status;

 (void)p;

  switch(sig) {

    case SIGCHLD: {

      // Reap all the childs
      while(1) {

        pid = waitpid(-1, &status, WNOHANG);
        if (pid < 0) {
          ERR("waitpid(): '%m' (%d)\n", errno);
          exit(1);
        }

        if (0 == pid) {
          // No more dead childs
          return;
        }

        // If it is our shell, exit...
        if (pid_sh == pid) {
          printf("%s: Exiting as the main shell#%d exited with status 0x%x (%d)\n",
                 REAPER_NAME,
                 pid,
                 status, status);

          exit(0);

        } else {

          printf("%s: Process#%d died with status 0x%x (%d)\n",
                 REAPER_NAME,
                 pid,
                 status, status);

        }
      } // End while
    }
    break;

    default: {
      printf("Received signal %s (%d) from process#%d\n", strsignal(sig), sig, info->si_pid);
    }
    break;

  } // End switch

} // sig_hdl



int main(int ac, char *av[])
{
  int              sig;
  int              rc;
  int              i;
  struct sigaction action;
  sigset_t         sigset;

  if (ac < 2) {
    fprintf(stderr, "Usage: %s sig1 sig2...\n", basename(av[0]));
    return 1;
  }

  for (i = 1; av[i]; i ++) {
    sig = sig_str2type(av[i]);
    if (sig > 0) {
      sigemptyset(&sigset);
      action.sa_sigaction = sig_hdl;
      action.sa_mask = sigset;
      action.sa_flags = SA_SIGINFO;
      printf("Capturing signal '%s' (%d)\n", av[i], sig);
      rc = sigaction(sig, &action, NULL);
      if (rc != 0) {
        ERR("sigaction(%s): '%m' (%d)\n", av[i], errno);
        return 1;
      }
    } else {
      fprintf(stderr, "Unknown signal '%s'\n", av[i]);
      return 1;
    }
  } // End for

  printf("%s#%d: forking a shell...\n", REAPER_NAME, getpid());

  // Create a child process for the shell
  pid_sh = fork();

  switch(pid_sh) {

    // Error
    case -1 : {
 
      fprintf(stderr, "fork(): '%m' (%d)\n", errno);
      return 1;

    }
    break;

    // Child
    case 0: {
      char *avs[2] = {
        "/bin/sh",
        NULL
      };

      extern char **environ;

      execve(avs[0], avs, environ);
      _exit(1);
    }
    break;

    // Father
    default : {

      // Main loop
      while(1) {

        pause();

      } // End while

    }
    break;

  } // End switch

  return 0;

} // main

