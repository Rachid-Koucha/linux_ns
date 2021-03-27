/*
  Semaphore undo ops after ipc_ns change

  Author: R. Koucha
  Date: 17-Fev-2020

*/

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <libgen.h>

#include "util.h"


static char *name;

static int undo = SEM_UNDO;




union semun {
               int              val;    /* Value for SETVAL */
               struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
               unsigned short  *array;  /* Array for GETALL, SETALL */
               struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                           (Linux-specific) */
           };

#define SEM_KEY 0x12345

static void semV_destroy(void)
{
  int id;

  id = semget(SEM_KEY, 0, 0);
  if (id >= 0) {
    (void)semctl(id, 0, IPC_RMID, NULL);
  }
} // semV_destroy


static int semV_init(void)
{
  union semun sop;
  int         id;
  int         rc;

  printf("%s is initializing semaphore\n", name);

  id = semget(SEM_KEY, 1, IPC_CREAT|IPC_EXCL|0777);
  if (id < 0) {
    ERR("semget(): '%m' (%d)\n", errno);
    return -1;
  }

  // Initialize semaphore to 1
  sop.val = 1;
  rc = semctl(id, 0, SETVAL, sop);
  if (rc != 0) {
    ERR("semctl(): '%m' (%d)\n", errno);
    return -1;
  }

  printf("%s got id of semaphore#%d\n", name, id);

  return id;
} // semV_init


static int semV_get(void)
{
  int         id;

  id = semget(SEM_KEY, 1, 0);
  if (id < 0) {
    ERR("semget(): '%m' (%d)\n", errno);
    return -1;
  }

  printf("%s got id of semaphore#%d\n", name, id);

  return id;
} // semV_get


static int P(int id)
{
  struct sembuf sem;
  int           rc;

  printf("%s: LOCKING semaphore#%d (SEM_UNDO %s)...\n", name, id, (undo ? "ON":"OFF"));

  sem.sem_num = 0;
  sem.sem_op = -1;
  sem.sem_flg = undo;
  rc = semop(id, &sem, 1);
  if (rc != 0) {
    ERR("semop(): '%m' (%d)\n", errno);
    return -1;
  }

  printf("%s: LOCKED semaphore#%d\n", name, id);

  return 0;
} // P


static int V(int id)
{
  struct sembuf sem;
  int           rc;

  printf("%s: UNLOCKING semaphore#%d (SEM_UNDO %s)...\n", name, id, (undo ? "ON":"OFF"));

  sem.sem_num = 0;
  sem.sem_op = 1;
  sem.sem_flg = undo;
  rc = semop(id, &sem, 1);
  if (rc != 0) {
    ERR("semop(): '%m' (%d)\n", errno);
    return -1;
  }

  printf("%s: UNLOCKED semaphore#%d...\n", name, id);

  return 0;
} // V


static void usage(char *pg)
{
  fprintf(stderr, "Usage: %s 0|1\n", basename(pg));
} // usage


int main(int ac, char *av[])
{
  int   id;
  int   rc;
  pid_t pid;
  int   sync[2];
  int   c;

  if ((ac != 2) || (av[1][1] != '\0')) {
    usage(av[0]);
    return 1;
  }

  switch(av[1][0]) {
    case '0': undo = 0; break;
    case '1': undo = SEM_UNDO; break;
    default : usage(av[0]); return 1;
  } // End switch

  name = "Father";

  // Create the synchronization pipe
  // sync[0] = read end
  // sync[1] = write end
  rc = pipe(sync);
  if (rc < 0) {
    ERR("pipe(): '%m' (%d)\n", errno);
    return 1;
  }

  // Remove any existing semaphore
  semV_destroy();

  // Get a semaphore identifier with initialization to 1
  id = semV_init();
  if (id < 0) {
    return 1;
  }

  // Father locks semaphore
  rc = P(id);
  if (rc != 0) {
    return 1;
  }

  // Create a child process
  pid = fork();
  if (pid == 0) {

    // Child process

    name = "Child";

    // Close the read end of the pipe
    close(sync[0]);

    // Get an identifier on the semaphore
    id = semV_get();
    if (id < 0) {
      ERR("semget(): '%m' (%d)\n", errno);
      return 1;
    }

    printf("%s write 1st message for father\n", name);
    c = 0;
    rc = write(sync[1], &c, sizeof(c));
    if (rc != sizeof(c)) {
      ERR("write(): '%m' (%d)\n", (rc < 0 ? errno : EIO));
      return 1;
    }

    // Lock the semaphore
    rc = P(id);
    if (rc != 0) {
      return 1;
    }

    printf("%s write 2nd message for father\n", name);
    c = 1;
    rc = write(sync[1], &c, sizeof(c));
    if (rc != sizeof(c)) {
      ERR("write(): '%m' (%d)\n", (rc < 0 ? errno : EIO));
      return 1;
    }

    // Unlock the semaphore
    rc = V(id);
    if (rc != 0) {
      return 1;
    }

    printf("Child exits\n");

    exit(0);

  } else {

    // Father process

    // Close the write end of the pipe
    close(sync[1]);

    printf("%s reading 1st message from child...\n", name);
    rc = read(sync[0], &c, sizeof(c));
    if (rc != sizeof(c)) {
      ERR("read(): '%m' (%d)\n", (rc < 0 ? errno : EIO));
      return 1;
    }
    printf("%s read 1st message from child\n", name);

    prompt("Father unshares ([Y]/N) ? ");
    c = getanswer();
    if ('\n' == c || 'y' == c || 'Y' == c) {
      printf("Father enters in new IPC namespace...\n");
      rc = unshare(CLONE_NEWIPC);
      if (0 != rc) {
        ERR("unshare(CLONE_NEWIPC): '%m' (%d)\n", errno);
        return 1;
      }
    } else {

      rc = V(id);
      if (rc != 0) {
        ERR("semop(): '%m' (%d)\n", errno);
        return 1;
      }

    }

    printf("%s reading 2nd message from child...\n", name);
    rc = read(sync[0], &c, sizeof(c));
    if (rc != sizeof(c)) {
      ERR("read(): '%m' (%d)\n", (rc < 0 ? errno : EIO));
      return 1;
    }
    printf("%s read 2nd message from child\n", name);

    // Close read end of the pipe
    close(sync[0]);

    rc = P(id);
    if (rc != 0) {
      return 1;
    }

    // Wait for the end of the child
    rc = waitpid(pid, 0, 0);
  }

  semV_destroy();

  return 0;

} // main
