/*
  Producer/Consumer sharing time through a system V message queue

  Author: R. Koucha
  Date: 28-Fev-2020

*/


#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <libgen.h>
#include <time.h>
#include <string.h>
#include <signal.h>

#include "util.h"


#define PRODUCER_NAME  "producer2"
#define CONSUMER_NAME  "consumer2"

static char *name;


#define MSG_KEY 0x12345

static  int     idq = -1;


static void sig_hdl(int s)
{
  int rc;

  printf("\n\n%s: Signal %d!\n\n", name, s);


  // If I am the producer ==> Unlink the message queue
  if (!strcmp(name, PRODUCER_NAME)) {

    if (idq != -1) {
      rc = msgctl(idq, IPC_RMID, 0);
      if (rc < 0) {
        ERR("msgctl(%d, IPC_RMID): '%m' (%d)\n", idq, errno);
        exit(1);
      }
    }
  }

  exit(0);

} // sig_hdl


int main(int ac, char *av[])
{
  int     rc;
  struct msgq {
    long mtype;
    char date[128];
  } msg;
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

    // Destroy any previously created message queue
    rc = msgget(MSG_KEY, 0);
    if (rc >= 0) {
      (void)msgctl(rc, IPC_RMID, 0);
    }

    // Get an identifier on a message queue
    idq = msgget(MSG_KEY, IPC_CREAT|IPC_EXCL|0777);
    if (idq < 0) {
      ERR("msgget(0x%x): '%m' (%d)\n", MSG_KEY, errno);
      return 1;
    }

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

      // Write current time in the message
      msg.mtype = 1;
      t = time(NULL);
      ctime_r(&t, msg.date);

      // Remove terminating '\n'
      msg.date[strlen(msg.date) - 1] = '\0';

      // Send the message
      rc = msgsnd(idq, &msg, sizeof(msg) - sizeof(msg.mtype), 0);
      if (rc != 0) {
        ERR("msgsnd(): '%m' (%d)\n", errno);
        return 1;
      }

    } // End while

  } else {

    /*
     * Consumer
     */

    // Get an identifier on the message queue
    idq = msgget(MSG_KEY, 0);
    if (idq < 0) {
      ERR("msgget(0x%x): '%m' (%d)\n", MSG_KEY, errno);
      return 1;
    }

    // Infinite loop
    while (1) {

      // Receive the message
      rc = msgrcv(idq, &msg, sizeof(msg) - sizeof(msg.mtype), 1, 0);
      if (rc < 0) {
        ERR("msgrcv(%d): '%m' (%d)\n", idq, errno);
        return 1;
      }

      // Print the date
      printf(" %s\r", msg.date);
      fflush(stdout);

    } // End while

  } // End if producer

  return  0;

} // main
