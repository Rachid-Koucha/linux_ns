#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>

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


static void sig_handler(
                        int sig, 
                        siginfo_t *info,
                        void *p
                       )
{
  (void)p;

  printf("Received signal %s (%d) from process#%d\n", strsignal(sig), sig, info->si_pid);

} // sig_handler





int main(int ac, char *av[])
{
  int              rc;
  int              i;
  int              sig;
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
      action.sa_sigaction = sig_handler;
      action.sa_mask = sigset;
      action.sa_flags = SA_SIGINFO;
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

  while (1) {
    pause();
  }

  return 0;

} // main
