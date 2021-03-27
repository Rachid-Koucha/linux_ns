/*

  Tool creating orphan processes

  Author: R. Koucha
  Date: 11-Apr-2020

*/


#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <libgen.h>
#include <stdlib.h>
#include <time.h>

#include "util.h"



int main(int ac, char *av[])
{
int i;
int nb;
int t;

  if (ac != 2) {
    fprintf(stderr, "Usage: %s nb_orphans\n", basename(av[0]));
    return 1;
  }

  if (!is_unsigned_integer(av[1])) {
    fprintf(stderr, "Specify a number of orphans instead of '%s'\n", av[1]);
    return 1;
  }

  nb = atoi(av[1]);

  // Set the seed
  srand(time(NULL));

  for (i = 0; i < nb; i ++) {
    t = rand() % 20;
    if (0 == fork()) {
      printf("Process#%d: Sleeping %d seconds...\n", getpid(), t);
      sleep(t);
      break;
    }
  } // End fork

  return 0;

} // main

