/*
  Creation of a POSIX message queue in several ipc_ns

  Author: R. Koucha
  Date: 16-Mar-2020

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
#include <string.h>

#include "util.h"



static char *name;


#define MQ_NAME   "/pmsg"
#define MQ_NAME1  "/pmsg1"

static mqd_t idq = (mqd_t)-1;
static mqd_t idq1 = (mqd_t)-1;



int main(int ac, char *av[])
{
  int     rc;
  struct mq_attr attr;
  struct {
    char dummy;
  } msg;
  int status;
  int fd;
  char path[256];

  (void)ac;

  snprintf(path, sizeof(path), "/proc/%d/ns/ipc", getpid());
  fd = open(path, O_RDONLY);
  if (fd < 0) {
    ERR("open(%s): '%m' (%d)\n", path, errno);
    return 1;
  }

  name = basename(av[0]);

  printf("%s#%d is starting...\n", name, getpid());

  // Remove any existing message queue
  (void)mq_unlink(MQ_NAME);
  (void)mq_unlink(MQ_NAME1);

  // Get an identifier on a message queue
  attr.mq_maxmsg = 2;
  attr.mq_msgsize = sizeof(msg);
  idq = mq_open(MQ_NAME, O_CREAT|O_EXCL|O_WRONLY, 0777, &attr);
  if (idq < 0) {
    ERR("mq_open(%s): '%m' (%d)\n", MQ_NAME, errno);
    return 1;
  }

  printf("%s: Created message queue %s\n\n# ls -l /dev/mqueue\n", name, MQ_NAME);

  status = system("ls -l /dev/mqueue");
  if (0 != status) {
    ERR("system(ls -l): '%m' (%d)\n", errno);
    return 1;
  }

  printf("\n");

  prompt("Unshare ipc_ns ? ");
  (void)getanswer();

  rc = unshare(CLONE_NEWIPC);
  if (rc < 0) {
    ERR("unshare(CLONE_NEWIPC): '%m' (%d)\n", errno);
    return 1;
  }

  printf("\n# ls -l /dev/mqueue\n");
  status = system("ls -l /dev/mqueue");
  if (0 != status) {
    ERR("system(ls -l): '%m' (%d)\n", errno);
    return 1;
  }

  printf("\n");

  prompt("Create msq queue '%s' ? ", MQ_NAME1);
  (void)getanswer();

  // Get an identifier on a message queue
  attr.mq_maxmsg = 2;
  attr.mq_msgsize = sizeof(msg);
  idq1 = mq_open(MQ_NAME1, O_CREAT|O_EXCL|O_WRONLY, 0777, &attr);
  if (idq1 < 0) {
    ERR("mq_open(%s): '%m' (%d)\n", MQ_NAME1, errno);
    return 1;
  }

  printf("\n%s: Created message queue %s\n\n# ls -l /dev/mqueue\n", name, MQ_NAME1);

  status = system("ls -l /dev/mqueue");
  if (0 != status) {
    ERR("system(ls -l): '%m' (%d)\n", errno);
    return 1;
  }

  printf("\n");

  prompt("Unshare mount_ns ? ");
  (void)getanswer();

  rc = unshare(CLONE_NEWNS);
  if (rc < 0) {
    ERR("unshare(CLONE_NEWNS): '%m' (%d)\n", errno);
    return 1;
  }

  printf("\n# ls -l /dev/mqueue\n");
  status = system("ls -l /dev/mqueue");
  if (0 != status) {
    ERR("system(ls -l): '%m' (%d)\n", errno);
    return 1;
  }

  printf("\n");

  prompt("Mount /dev/mqueue (mount --make-rslave /; mount -t mqueue mqueue /dev/mqueue) ? ");
  (void)getanswer();

  status = system("mount --make-rslave /; mount -t mqueue mqueue /dev/mqueue");
  if (0 != status) {
    ERR("system(): '%m' (%d)\n", errno);
    return 1;
  }

  printf("\n# ls -l /dev/mqueue\n");
  status = system("ls -l /dev/mqueue");
  if (0 != status) {
    ERR("system(ls -l): '%m' (%d)\n", errno);
    return 1;
  }

  printf("\n");

  prompt("End ? ");
  (void)getanswer();  

  // Cleanup
  (void)mq_unlink(MQ_NAME1);
  rc = setns(fd, 0);
  if (rc < 0) {
    ERR("setns(): '%m' (%d)\n", errno);
    return 1;
  }
  (void)mq_unlink(MQ_NAME);

  return  0;

} // main
