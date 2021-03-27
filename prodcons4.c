/*
  Producer/Consumer sharing time through a POSIX shared memory segment

  Author: R. Koucha
  Date: 17-Mar-2020

*/


#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <libgen.h>
#include <time.h>
#include <string.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "util.h"


#define PRODUCER_NAME  "producer4"
#define CONSUMER_NAME  "consumer4"

static char *name;


#define SHM_NAME "/mdate"
#define SHM_SIZE 4096
#define SEM_NAME "/sdate"

static int    idm = -1;
static char  *pShm;
static sem_t *ids = SEM_FAILED;


static void clean_ipc(void)
{
  int rc;

  if (idm >= 0) {
    rc = close(idm);
    if (rc != 0) {
      ERR("close(%d): '%m' (%d)\n", idm, errno);
      exit(1);
    }
  }

  if (pShm) {
    rc = munmap(pShm, SHM_SIZE);
    if (rc != 0) {
      ERR("munmap(%p): '%m' (%d)\n", pShm, errno);
      exit(1);
    }
    pShm = NULL;
  }

  rc = shm_unlink(SHM_NAME);
  if (rc != 0) {
    ERR("shm_unlink(%s): '%m' (%d)\n", SHM_NAME, errno);
    exit(1);
  }

  if (ids != SEM_FAILED) {
    rc = sem_close(ids);
    if (rc != 0) {
      ERR("sem_close(): '%m' (%d)\n", errno);
      exit(1);
    }

    rc = sem_unlink(SEM_NAME);
    if (rc != 0) {
      ERR("sem_unlink(%s): '%m' (%d)\n", SEM_NAME, errno);
      exit(1);
    }
  }

} // clean_ipc


static void sig_hdl(int s)
{
  int rc;

  printf("\n\n%s#%d: Signal %d!\n\n", name, getpid(), s);

  // If I am the producer ==> Clean all IPCs
  if (!strcmp(name, PRODUCER_NAME)) {
    clean_ipc();
  } else {

    // Consumer ==> Close all

    if (idm >= 0) {
      rc = close(idm);
      if (rc != 0) {
        ERR("close(%d): '%m' (%d)\n", idm, errno);
        exit(1);
      }
    }

    if (ids != SEM_FAILED) {
      rc = sem_close(ids);
      if (rc != 0) {
        ERR("sem_close(): '%m' (%d)\n", errno);
        exit(1);
      }
    }
  } // End if producer

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


static void usage(const char *pg)
{
  fprintf(stderr, "Usage: %s [-d]\n", pg);
}

int main(int ac, char *av[])
{
  int     rc;
  time_t  t;
  int cleanup = 0;

  name = basename(av[0]);

  if (!strcmp(CONSUMER_NAME, name) && (ac != 1)) {
    fprintf(stderr, "%s does not accept parematers\n", name);
    return 1;
  }

  if (ac > 2) {
    usage(name);
    return 1;
  }

  if (ac == 2) {
    if (strcmp("-d", av[1])) {
      usage(name);
      return 1;
    }
    cleanup = 1;
  }

  // Capture CTRL-C
  (void)signal(SIGINT, sig_hdl);

  printf("%s#%d is starting...\n", name, getpid());

  // If I am the producer
  if (!strcmp(name, PRODUCER_NAME)) {

    /*
     * Producer
     */

    // Destroy any previously created memory segment if requested
    idm = shm_open(SHM_NAME, O_RDWR, 0);
    if (idm >= 0) {
      if (cleanup) {
        clean_ipc();

        // Get an identifier on a shared memory segment
        idm = shm_open(SHM_NAME, O_RDWR|O_CREAT, 0777);
        if (idm < 0) {
          ERR("shm_open(%s): '%m' (%d)\n", SHM_NAME, errno);
          return 1;
        }
      } else {
        fprintf(stderr, "%s already exists ?!?\n", SHM_NAME);
        return 1;
      }
    } else {

      // Get an identifier on a shared memory segment
      idm = shm_open(SHM_NAME, O_RDWR|O_CREAT, 0777);
      if (idm < 0) {
        ERR("shm_open(%s): '%m' (%d)\n", SHM_NAME, errno);
        return 1;
      }
    }

    // Destroy any previously created semaphore if requested
    ids = sem_open(SEM_NAME, O_RDWR);
    if (ids != SEM_FAILED) {
      if (cleanup) {

        rc = sem_close(ids);
        if (rc != 0) {
          ERR("sem_close(): '%m' (%d)\n", errno);
          return 1;
        }

        rc = sem_unlink(SEM_NAME);
        if (rc != 0) {
          ERR("sem_unlink(%s): '%m' (%d)\n", SEM_NAME, errno);
          return 1;
        }

        // Get an identifier on a semaphore
        ids = sem_open(SEM_NAME, O_RDWR|O_CREAT, 0777, 1);
        if (ids == SEM_FAILED) {
          ERR("sem_open(%s): '%m' (%d)\n", SEM_NAME, errno);
          return 1;
        }
      } else {
        fprintf(stderr, "%s already exists ?!?\n", SEM_NAME);
        return 1;
      }
    } else {

        // Get an identifier on a semaphore
        ids = sem_open(SEM_NAME, O_RDWR|O_CREAT, 0777, 1);
        if (ids == SEM_FAILED) {
          ERR("sem_open(%s): '%m' (%d)\n", SEM_NAME, errno);
          return 1;
        }
    }

    // Set the size of the memory segment
    rc = ftruncate(idm, SHM_SIZE);
    if (rc < 0) {
      ERR("ftruncate(%d, %zu): '%m' (%d)\n", idm, (size_t)SHM_SIZE, errno);
      return 1;
    }

    // Attach the memory segment to the address space
    pShm = mmap(0, SHM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, idm, 0);
    if (MAP_FAILED == pShm) {
      ERR("mmap(): '%m' (%d)\n", errno);
      return 1;
    }

    // No longer needed after mmap()
    close(idm);
    idm = -1;

    // Reset the zone
    memset(pShm, 0, SHM_SIZE);

    // Infinite loop
    while (1) {

      rc = P(ids);
      if (rc != 0) {
        return 1;
      }

      // Write current time in the shared memory segment
      t = time(NULL);
      ctime_r(&t, pShm);

      // Remove terminating '\n'
      *(pShm + strlen(pShm) - 1) = '\0';
      
      rc = V(ids);
      if (rc != 0) {
        return 1;
      }

    } // End while

  } else {

    /*
     * Consumer
     */

    // Get an identifier on a shared memory segment
    idm = shm_open(SHM_NAME, O_RDWR, 0);
    if (idm < 0) {
      ERR("shm_open(%s): '%m' (%d)\n", SHM_NAME, errno);
      return 1;
    }

    // Get an identifier on a semaphore
    ids = sem_open(SEM_NAME, O_RDWR);
    if (ids == SEM_FAILED) {
      ERR("sem_open(%s): '%m' (%d)\n", SEM_NAME, errno);
      return 1;
    }

    // Attach the memory segment to the virtual address space
    pShm = mmap(0, SHM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, idm, 0);
    if (MAP_FAILED == pShm) {
      ERR("mmap(): '%m' (%d)\n", errno);
      return 1;
    }

    // No longer needed after mmap()
    close(idm);
    idm = -1;

    // Infinite loop
    while (1) {

      sleep(1);

      rc = P(ids);
      if (rc != 0) {
        return 1;
      }

      // If there is something to read in the shared memory segment
      if (pShm[0]) {
        printf(" %s\r", pShm);
        fflush(stdout);

        // Reset the zone
        memset(pShm, 0, SHM_SIZE);
      }

      rc = V(ids);
      if (rc != 0) {
        return 1;
      }
    } // End while

  } // End if producer

  return  0;

} // main
