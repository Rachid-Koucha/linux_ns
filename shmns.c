
#include <errno.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sched.h>


#include "util.h"



#define SHM_KEY  0x123456
#define SHM_SIZE 4096









int main(void)
{
int   id;
int   rc;
char *p;
int   c;

  // Destroy any previously created segment
  rc = shmget(SHM_KEY, 0, 0);
  if (rc >= 0) {
    (void)shmctl(rc, IPC_RMID, 0);
  }

  // Get an identifier on a shared memory segment
  id = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT|IPC_EXCL|0777);
  if (id < 0) {
    ERR("shmget(): '%m' (%d)\n", errno);
    return 1;
  }

  prompt("Unshare before shmat() ([Y]/N) ? ");
  c = getanswer();
  if (c == 'Y' || c == 'y' || c == '\n') {
    rc = unshare(CLONE_NEWIPC);
    if (rc < 0) {
      ERR("unshare(): '%m' (%d)\n", errno);
      return 1;
    }
  }

  // Attach the segment to the address space
  p = shmat(id, 0, 0777);
  if (p == (char *)-1) {
    ERR("shmat(): '%m' (%d)\n", errno);
    return 1;
  }

  snprintf(p, SHM_SIZE, "Data in the shared memory segment\n");

  printf("%s", p);

  // Detach the memory segment
  rc = shmdt(p);
  if (0 != rc) {
    ERR("shmdt(): '%m' (%d)\n", errno);
    return 1;
  }

  // Destroy the memory segment
  shmctl(id, IPC_RMID, 0);

  return 0;

} // main
