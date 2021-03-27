/*

  Tool displaying the pid/ppid of the current process

  Author: R. Koucha
  Date: 06-Feb-2020

*/

#include <errno.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>


int main(void)
{
  printf("PID#%d, PPID#%d\n", getpid(), getppid());
  return 0;
} // main
