/*

  Tool displaying information about a file system

  Author: R. Koucha
  Date: 06-Apr-2020

*/


#include <errno.h>
#include <sys/types.h>
#include <stdio.h>
#include <libgen.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/sysmacros.h>
#include <sys/vfs.h>
#include <sys/mount.h>
#include <linux/magic.h>

#include "util.h"



static const char *get_mntflags(unsigned long flags)
{
static char  flag_str[1024];
unsigned int offset = 0;
int          strsize = sizeof(flag_str);
int          rc;

#define WRITE_FLAG(str, strsize, off, flag) do {	\
           if (off) {   \
             str[off] = ' '; \
             off ++; \
             strsize --; \
           } \
           rc = snprintf(str + off, strsize, "%s", flag);	\
           off += rc; \
           strsize -= rc; \
     } while(0)

  flag_str[0] = '\0';

  if (flags & MS_MANDLOCK) {
    WRITE_FLAG(flag_str, strsize, offset, "MANDATORY_LOCKING");
  }

  if (flags & MS_NOATIME) {
    WRITE_FLAG(flag_str, strsize, offset, "NOATIME");
  }

  if (flags & MS_NODEV) {
    WRITE_FLAG(flag_str, strsize, offset, "NODEV");
  }

  if (flags & MS_NODIRATIME) {
    WRITE_FLAG(flag_str, strsize, offset, "NODIRATIME");
  }

  if (flags & MS_NOEXEC) {
    WRITE_FLAG(flag_str, strsize, offset, "NOEXEC");
  }

  if (flags & MS_NOSUID) {
    WRITE_FLAG(flag_str, strsize, offset, "NOSUID");
  }

  if (flags & MS_RDONLY) {
    WRITE_FLAG(flag_str, strsize, offset, "RDONLY");
  }

  if (flags & MS_RELATIME) {
    WRITE_FLAG(flag_str, strsize, offset, "RELATIME");
  }

  if (flags & MS_SYNCHRONOUS) {
    WRITE_FLAG(flag_str, strsize, offset, "SYNCHRONOUS");
  }

  if (flags & MS_REMOUNT) {
    WRITE_FLAG(flag_str, strsize, offset, "REMOUNT");
  }

  if (flags & MS_BIND) {
    WRITE_FLAG(flag_str, strsize, offset, "BIND");
  }

  if (flags & MS_SLAVE) {
    WRITE_FLAG(flag_str, strsize, offset, "SLAVE");
  }

  if (flags & MS_SHARED) {
    WRITE_FLAG(flag_str, strsize, offset, "SHARED");
  }

  if (flags & MS_PRIVATE) {
    WRITE_FLAG(flag_str, strsize, offset, "PRIVATE");
  }

  if (flags & MS_UNBINDABLE) {
    WRITE_FLAG(flag_str, strsize, offset, "UNBINDABLE");
  }

  return flag_str;
} // get_mntflags


static const char *get_fstype(unsigned long fstype)
{

  switch(fstype)
  {
    case RAMFS_MAGIC: return "RAMFS";
    case TMPFS_MAGIC: return "TMPFS";
    case HUGETLBFS_MAGIC : return "HUGETLBFS";
    //case EXT2_SUPER_MAGIC:
    //case EXT3_SUPER_MAGIC:
    case EXT4_SUPER_MAGIC: return "EXT2/3/4FS";
    case BTRFS_SUPER_MAGIC: return "BTRFS";
    case OVERLAYFS_SUPER_MAGIC: return "OVERLAYFS";
    case CGROUP_SUPER_MAGIC: return "CGROUPV1";
    case CGROUP2_SUPER_MAGIC: return "CGROUPV2";
    case DEVPTS_SUPER_MAGIC: return "DEVPTSFS";
    case PROC_SUPER_MAGIC: return "PROCFS";
    case SYSFS_MAGIC: return "SYSFS";
    case NSFS_MAGIC: return "NSFS";
    default: return "????";
  } // End switch

} // get_fstype


int main(int ac, char *av[])
{
  int           rc;
  struct statfs st;

  if (ac != 2) {
    fprintf(stderr, "Usage: %s fname\n", basename(av[0]));
    return 1;
  }

  // Get information on the file system
  rc = statfs(av[1], &st);
  if (0 != rc) {
    ERR("statfs(%s): '%m' (%d)\n", av[1], errno);
    return 1;
  }

  printf("Type             : 0x%lx (%s)\n"
         "Block size       : %lu\n"
         "Total blocks     : %lu\n"
         "Total free blocks: %lu\n"
         "Total files      : %lu\n"
         "Max name length  : %lu\n"
         "Mount flags      : 0x%lx (%s)\n"
         ,
         (unsigned long)(st.f_type), get_fstype((unsigned long)(st.f_type)),
         (unsigned long)(st.f_bsize),
         (unsigned long)(st.f_blocks),
         (unsigned long)(st.f_bfree),
         (unsigned long)(st.f_files),
         (unsigned long)(st.f_namelen),
         (unsigned long)(st.f_flags), get_mntflags((unsigned long)(st.f_flags))
        );

  return 0;

} // main



