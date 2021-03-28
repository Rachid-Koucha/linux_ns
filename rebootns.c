/*
   Command to reboot with a given parameter

   Author: R. Koucha
   Date: 25-Mar-2020
  
*/

#include <errno.h>
#include <stdio.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/reboot.h>
#include <string.h>


static struct {
  const char *str;
  int         type;
} reboot_type[] = {

  { "CAD_OFF",     RB_DISABLE_CAD },
  { "CAD_ON",      RB_ENABLE_CAD  },
  { "HALT_SYSTEM", RB_HALT_SYSTEM },
  { "KEXEC",       RB_KEXEC       },
  { "POWER_OFF",   RB_POWER_OFF   },
  { "AUTOBOOT",    RB_AUTOBOOT    },
  { "SW_SUSPEND",  RB_SW_SUSPEND  },
  { NULL,          0              }

};


static void display_types(void)
{
int i;

  fprintf(stderr, "\nreboot param is one of:\n\n");
  for (i = 0; reboot_type[i].str; i ++) {
    fprintf(stderr, "\t\t%s\n", reboot_type[i].str);
  } // End for

} // display_types


static int str2cmd(const char *cmd)
{
int i;

  for (i = 0; reboot_type[i].str; i ++) {
    if (!strcmp(reboot_type[i].str, cmd)) {
      return reboot_type[i].type;
    }
  } // End for

  return -1;

} // str2cmd


int main(int ac, char *av[])
{
  int rc;
  int cmd;

  if (ac != 2) {
    fprintf(stderr, "Usage: %s reboot_param\n", basename(av[0]));
    display_types();
    return 1;
  }

  cmd = str2cmd(av[1]);

  if (cmd >= 0) {
    /* Under glibc and most alternative libc's (including uclibc, dietlibc,
       musl and a few others), some of the constants involved have gotten
       symbolic names RB_*, and the library call is a 1-argument
       wrapper around the system call: */
    rc = reboot(cmd);
  } else {
    errno = EINVAL;
    rc = -1;
  }

  if (0 == rc) {
    return rc;
  }

  fprintf(stderr, "reboot(%s): '%m' (%d)\n", av[1], errno);

  return 1;

} // main
