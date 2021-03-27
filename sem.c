/*

  Simple program to create a SYS V semaphore

  Author: R. Koucha
  Date: 25-Mar-2020

*/



#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>


#include "util.h"



int main(void)
{
  int rc;

  rc = semget(0x12345, 1, IPC_CREAT|IPC_EXCL);
  if (rc < 0) {
    ERR("semget(): '%m' (%d)\n", errno);
    return 1;
  }

  printf("Semaphore id is: %d\n", rc);

  pause();

  return 0;

} // main
