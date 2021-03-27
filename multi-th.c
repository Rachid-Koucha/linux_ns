#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"


static pthread_t tid[256];

static void *th_main(void *param)
{
int i = (int)((pthread_t *)param - tid);

  printf("Thread %d is running...\n", i);

  pause();

  printf("Thread %d is terminating...\n", i);

  return NULL;  
} // th_main


int main(int ac, char *av[])
{
int nth;
int rc;
int i;
void *p;

  if (ac != 2)
  {
    return 1;
  }

  nth = atoi(av[1]);
  if (nth > (int)(sizeof(tid)/sizeof(pthread_t)))
  {
    fprintf(stderr, "Too many threads (max = %zu)\n", sizeof(tid)/sizeof(pthread_t));
    return 1;
  }

  printf("Creating %d threads\n", nth);

  for (i = 0; i < nth; i ++)
  {
    p = (void *)&(tid[i]);
    rc = pthread_create(&(tid[i]), NULL, th_main, p);
    if (0 != rc)
    {
      errno = rc;
      ERR("pthread_create(%d): '%m' (%d)\n", i, errno);
      return 1;
    }
  } // End for

  printf("Waiting...\n");

  pause();

  printf("Exiting...\n");

  return 0;

} // main

