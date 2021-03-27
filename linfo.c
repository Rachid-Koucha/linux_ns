/*

  Tool displaying information about a symbolic link

  Author: R. Koucha
  Date: 05-Apr-2020

*/


#include <errno.h>
#include <sys/types.h>
#include <stdio.h>
#include <libgen.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/sysmacros.h>

#include "util.h"



int main(int ac, char *av[])
{
  int             rc;
  struct stat     stl, stt;
  ssize_t         ssz;
  char            lname[256];

  if (ac != 2) {
    fprintf(stderr, "Usage: %s fname\n", basename(av[0]));
    return 1;
  }

  // Get information on the symbolic link
  rc = lstat(av[1], &stl);
  if (0 != rc) {
    ERR("lstat(%s): '%m' (%d)\n", av[1], errno);
    return 1;
  }

  // Check that it is a symbolic link
  if (((stl.st_mode & S_IFMT) & S_IFLNK) != S_IFLNK) {
    ERR("File '%s' is not a symbolic link\n", av[1]);
    return 1;
  }

  // Get the information on the target
  rc = stat(av[1], &stt);
  if (0 != rc) {
    ERR("stat(%s): '%m' (%d)\n", av[1], errno);
    return 1;
  }

  // Get the name of the target
  memset(lname, 0, sizeof(lname));
  ssz = readlink(av[1], lname, sizeof(lname));
  if (-1 == ssz) {
    ERR("readlink(%s): '%m' (%d)\n", av[1], errno);
    return 1;
  }

  printf("Symbolic link:\n"
         "\tName: %s\n"
         "\tRights: 0%o\n"
         "\tDevice (major/minor): 0x%x/0x%x\n"
         "\tInode: 0x%x (%u)\n"
         "Target:\n"
         "\tName: %s\n"
         "\tRights: 0%o\n"
         "\tDevice (major/minor): 0x%x/0x%x\n"
         "\tInode: 0x%x (%u)\n"
         ,
         av[1],
         stl.st_mode & 0777,
         major(stl.st_dev), minor(stl.st_dev),
         (unsigned int)(stl.st_ino), (unsigned int)(stl.st_ino),
         lname,
         stt.st_mode & 0777,
         major(stt.st_dev), minor(stt.st_dev),
         (unsigned int)(stt.st_ino), (unsigned int)(stt.st_ino)
        );

  return 0;

} // main



