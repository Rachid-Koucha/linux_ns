/*
  Producer/Consumer sharing time through a POSIX message queue

  Author: R. Koucha
  Date: 09-Mar-2020

*/


#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h> 
#include <mqueue.h>
#include <sys/msg.h>
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <libgen.h>
#include <time.h>
#include <string.h>
#include <signal.h>

#include "util.h"



#define PRODUCER_NAME  "producer3"
#define CONSUMER_NAME  "consumer3"

static char *name;


#define MQ_NAME   "/qdate"

static mqd_t idq = (mqd_t)-1;



static void sig_hdl(int s)
{
  int rc;

  printf("\n\n%s: Signal %d!\n\n", name, s);

  if (idq != (mqd_t)-1) {

    rc = mq_close(idq);
    if (rc < 0) {
      ERR("mq_close(): '%m' (%d)\n", errno);
      exit(1);
    }
  }

  // If I am the producer ==> Unlink the message queue
  if (!strcmp(name, PRODUCER_NAME)) {

    rc = mq_unlink(MQ_NAME);
    if (rc < 0) {
      ERR("mq_unlink(%s): '%m' (%d)\n", MQ_NAME, errno);
      exit(1);
    }
  }

  exit(0);

} // sig_hdl




int main(int ac, char *av[])
{
  int     rc;
  struct msgq {
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

    int            already_unshared = 0;
    struct mq_attr attr;

    /*
     * Producer
     */

    // Remove any existing message queue
    (void)mq_unlink(MQ_NAME);

    // Get an identifier on a message queue
    attr.mq_maxmsg = 2;
    attr.mq_msgsize = sizeof(msg);
    idq = mq_open(MQ_NAME, O_CREAT|O_EXCL|O_WRONLY, 0777, &attr);
    if (idq < 0) {
      ERR("mq_open(%s): '%m' (%d)\n", MQ_NAME, errno);
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
      t = time(NULL);
      ctime_r(&t, msg.date);

      // Remove terminating '\n'
      msg.date[strlen(msg.date) - 1] = '\0';

      // Send the message
      rc = mq_send(idq, (char *)&msg, sizeof(msg), 0);
      if (rc != 0) {
        ERR("mq_send(): '%m' (%d)\n", errno);
        return 1;
      }

    } // End while

  } else {

    /*
     * Consumer
     */

    unsigned int prio;
    ssize_t      ssz;

    // Get an identifier on the message queue
    idq = mq_open(MQ_NAME, O_RDONLY);
    if (idq < 0) {
      ERR("mq_open(%s): '%m' (%d)\n", MQ_NAME, errno);
      return 1;
    }

    // Infinite loop
    while (1) {

      // Receive the message
      ssz = mq_receive(idq, (char *)&msg, sizeof(msg), &prio);
      if (ssz < 0) {
        ERR("mq_receive(%d): '%m' (%d)\n", idq, errno);
        return 1;
      }

      // Print the date
      printf(" %s\r", msg.date);
      fflush(stdout);

    } // End while

  } // End if producer

  return  0;

} // main
