/*
  Producer/Consumer sharing time through a shared memory segment

  Author: R. Koucha
  Date: 28-Fev-2020

*/


#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <libgen.h>
#include <time.h>
#include <string.h>
#include <semaphore.h>
#include <signal.h>

#include "util.h"


#define PRODUCER_NAME  "producer"
#define CONSUMER_NAME  "consumer"

static char *name;


#define SHM_KEY 0x12345
#define SHM_SIZE 4096

static int   idm = -1;
static char *pShm;

static void sig_hdl(int s)
{
  int rc;

  printf("\n\n%s: Signal %d!\n\n", name, s);

  if (pShm) {
    rc = shmdt(pShm);
    if (rc != 0) {
      ERR("shmdt(%p): '%m' (%d)\n", pShm, errno);
      exit(1);
    }
  }

  // If I am the producer ==> Unlink the message queue
  if (!strcmp(name, PRODUCER_NAME)) {

    if (idm != -1) {
      rc = shmctl(idm, IPC_RMID, 0);
      if (rc < 0) {
        ERR("shmctl(%d, IPC_RMID): '%m' (%d)\n", idm, errno);
        exit(1);
      }
    }
  }

  exit(0);

} // sig_hdl



static int P(sem_t *sem)
{
  int rc;

  rc = sem_wait(sem);
  if (rc != 0) {
    ERR("sem_wait(): '%m' (%d)\n", errno);
    return -1;
  }

  return 0;
} // P


static int V(sem_t *sem)
{
  int rc;

  rc = sem_post(sem);
  if (rc != 0) {
    ERR("sem_post(): '%m' (%d)\n", errno);
    return -1;
  }

  return 0;
} // V


int main(int ac, char *av[])
{
  int     rc;
  char   *pData;
  size_t  data_sz;
  int     c;
  time_t  t;

  (void)ac;

  name = basename(av[0]);

  printf("%s#%d is starting...\n", name, getpid());

  // Capture CTRL-C
  (void)signal(SIGINT, sig_hdl);

  // If I am the producer
  if (!strcmp(name, PRODUCER_NAME)) {

    int already_unshared = 0;

    /*
     * Producer
     */

    // Destroy any previously created memory segment
    rc = shmget(SHM_KEY, 0, 0);
    if (rc >= 0) {
      (void)shmctl(rc, IPC_RMID, 0);
    }

    // Get an identifier on a shared memory segment
    idm = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT|IPC_EXCL|0777);
    if (idm < 0) {
      ERR("shmget(0x%x): '%m' (%d)\n", SHM_KEY, errno);
      return 1;
    }

    // Attach the memory segment to the virtual address space
    pShm = shmat(idm, 0, 0777);
    if (pShm == (char *)-1) {
      ERR("shmat(%d): '%m' (%d)\n", idm, errno);
      return 1;
    }

    printf("%s: Shared memory segment attached at: %p\n", name, pShm);

    // Semaphore creation and initialization to 1
    rc = sem_init((sem_t *)pShm, 1, 1);
    if (rc < 0) {
      return 1;
    }

    // Data zone (right after the semaphore)
    pData = (char *)(((sem_t *)pShm) + 1);
    data_sz = (pShm + SHM_SIZE) - pData;

    // Reset the zone
    memset(pData, 0, data_sz);

    // Infinite loop
    while (1) {

      if (!already_unshared) {
        prompt("Unshare ipc namespaces ([Y]/N) ? ");
        c = getanswer();
        if (c == 'Y' || c == 'y' || c == '\n') {
          rc = unshare(CLONE_NEWIPC);
          if (rc < 0) {
            ERR("unshare(): '%m' (%d)\n", errno);
            return 1;
          }

          already_unshared = 1;
        }
      } else {
        prompt("Update time ([Y]/N) ? ");
        c = getanswer();
        if (c != 'Y' && c != 'y' && c != '\n') {
          continue;
        }
      }

      rc = P((sem_t *)pShm);
      if (rc != 0) {
        return 1;
      }

      // Write current time in the shared memory segment
      t = time(NULL);
      ctime_r(&t, pData);

      // Remove terminating '\n'
      *(pData + strlen(pData) - 1) = '\0';
      
      rc = V((sem_t *)pShm);
      if (rc != 0) {
        return 1;
      }

    } // End while

  } else {

    /*
     * Consumer
     */

    // Get an identifier on a shared memory segment
    idm = shmget(SHM_KEY, SHM_SIZE, 0);
    if (idm < 0) {
      ERR("shmget(0x%x): '%m' (%d)\n", SHM_KEY, errno);
      return 1;
    }

    // Attach the memory segment to the virtual address space
    pShm = shmat(idm, 0, 0777);
    if (pShm == (char *)-1) {
      ERR("shmat(%d): '%m' (%d)\n", idm, errno);
      return 1;
    }

    printf("%s: Shared memory segment attached at %p\n", name, pShm);

    // Data zone (right after the semaphore)
    pData = (char *)(((sem_t *)pShm) + 1);
    data_sz = (pShm + SHM_SIZE) - pData;

    // Infinite loop
    while (1) {

      sleep(1);

      rc = P((sem_t *)pShm);
      if (rc != 0) {
        return 1;
      }

      // If there is something to read in the shared memory segment
      if (pData[0]) {
        printf(" %s\r", pData);
        fflush(stdout);

        // Reset the zone
        memset(pData, 0, data_sz);
      }

      rc = V((sem_t *)pShm);
      if (rc != 0) {
        return 1;
      }
    } // End while

  } // End if producer

  return  0;

} // main
