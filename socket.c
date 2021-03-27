/*

  Tool creating a socket and displaying the identity of the
  network namespace to which it belongs to

*/



#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/sockios.h>  // For SIOCGSKNS
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "util.h"






int main(void)
{
int         sd;
int         fd;
struct stat st;
int         rc;

  // Create a socket
  sd = socket(AF_UNIX, SOCK_DGRAM, 0);
  if (sd < 0) {
    ERR("socket(): '%m' (%d)\n", errno);
    return 1;
  }

  // Get a file descriptor on its network_ns
  fd = ioctl(sd, SIOCGSKNS);
  if (fd < 0) {
    ERR("ioctl(SIOCGSKNS): '%m' (%d)\n", errno);
    return 1;
  }

  // Get the information about the network_ns
  rc = fstat(fd, &st);
  if (rc < 0) {
    ERR("fstat(): '%m' (%d)\n", errno);
    return 1;
  }

  printf("Network namespace is [Device,Inode]: [%lu,%lu]\n", st.st_dev, st.st_ino);

  close(sd);
  close(fd);

  return 0;

} // main
