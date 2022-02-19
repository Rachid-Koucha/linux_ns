/*

  Tool running a shell in new namespaces in unprivileged mode

  Author: R. Koucha
  Date: 20-Fev-2020

*/

#include <errno.h>
#include <sys/types.h>
#include <stdio.h>
#include <libgen.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <getopt.h>

#include "util.h"



int dbg_level;



// ----------------------------------------------------------------------------
// Name   : shns_longops
// Usage  : Option on the command line
// ----------------------------------------------------------------------------
static struct option shns_longopts[] =
{
  { "shell",                required_argument, NULL, 's' },
  { "umap",                 required_argument, NULL, 'u' },
  { "gmap",                 required_argument, NULL, 'g' },
  { "nspc",                 required_argument, NULL, 'n' },
  { "debug",                required_argument, NULL, 'd' },
  { "help",                 no_argument,       NULL, 'h' },

  // Last entry
  {NULL, 0, NULL, 0 }
};



#define DEFAULT_CMD   "/bin/sh"








#define NB_NS_NAME 7

struct ns_list_t {
  char *name;
  int   selected;
  int   flag;
} ns_list[NB_NS_NAME] = {
  { "ipc",    0, CLONE_NEWIPC    },
#define SHNS_IPC_IDX 0
  { "pid",    0, CLONE_NEWPID    },
#define SHNS_PID_IDX 1
  { "net",    0, CLONE_NEWNET    },
#define SHNS_NET_IDX 2
  { "user",   0, CLONE_NEWUSER   },
#define SHNS_USER_IDX 3
  { "uts",    0, CLONE_NEWUTS    },
#define SHNS_UTS_IDX 4
  { "cgroup", 0, CLONE_NEWCGROUP },
#define SHNS_CGROUP_IDX 5
  { "mnt",    0, CLONE_NEWNS     }
#define SHNS_MNT_IDX 6
};



static int add_map(
                   char ***map,
                   int    *nmap,
                   char   *new
                  )
{
  (*nmap) += 1;
  *map = (char **)realloc(*map, (*nmap) * sizeof(char *));
  if (!(*map)) {
    return -1;
  }

  (*map)[(*nmap) - 1] = strdup(new);
  if (!((*map)[(*nmap) - 1])) {
    return -1;
  }

  return 0;

} // add_map


static void free_map(
                   char **map,
                   int    nmap
                  )
{
  int i;

  for (i = 0; i < nmap; i ++) {
    free(map[i]);
  }

  if (nmap) {
    free(map);
  }
} // free_map


static char *make_cmdline(
			  char  *cmd,
                          pid_t  pid,
                          char **map,
                          int    nmap
			 )
{
char   *cmdline = (char *)0;
size_t  l;
int     i;
char   *p;
int     rc;

  // new[gi]udmap pid x x x...

  // Command
  l = strlen(cmd) + 1;

  // pid
  l += 8 + 1;

  // The maps (the latter "+1 is for terminating NULL char)
  for (i = 0; i < nmap; i ++) {
    l += strlen(map[i]) + 1;
  } // End for

  cmdline = (char *)malloc(l);
  if (!cmdline) {
    return NULL;
  }

  rc = snprintf(cmdline, l, "%s %d ", cmd, pid);
  l -= rc;

  p = cmdline + rc;

  for (i = 0; i < nmap; i ++) {

    if (i < (nmap - 1)) {
      rc = snprintf(p, l, "%s ", map[i]);
    } else {
      rc = snprintf(p, l, "%s", map[i]);
    }

    p += rc;
    l -= rc;
  } // End for

  return cmdline;

} // make_cmdline



static int run_cmdline(char *cmdline)
{
  int status;

  status = system(cmdline);

  if (WIFEXITED(status)) {
    return WEXITSTATUS(status);
  } else {
    return -1;
  }

} // run_cmdline




// Set the "selected" field in ns_list[] for the given
// namespace or all (if keyword "all" is passed)
static int select_ns(const char *ns)
{
  int i, fnd;

  fnd = 0;
  for (i = 0; i < NB_NS_NAME; i ++) {
    if (!strcmp("all", ns)) {
      ns_list[i].selected = 1;
      fnd = 1;
    } else {
      if (!strcmp(ns_list[i].name, ns)) {
        ns_list[i].selected = 1;
        return 0;
      }
    }
  } // End for

  // Bad namespace name
  if (!fnd) {
    return -1;
  }

  return 0;

} // select_ns


// ----------------------------------------------------------------------------
// Name   : usage
// Usage  : Display the help of this program
// Return : None
// ----------------------------------------------------------------------------
static void usage(char *cmd)
{
  fprintf(stderr,
          "Usage: %s [-h] [-d level] [-n nsname] [-u uidmap] [-g gidmap] [-s path]\n"
          "\n"
          "  -h|--help        : This help\n"
          "  -d|--debug level : Set debug to 'level'\n"
          "  -n|--nspc nsname : Create namespace 'nsname'\n"
          "                     'nsname' is: cgroup|ipc|net|mnt|pid|user|uts\n"
          "  -u|--umap uidmap : User id mapping\n"
          "                     'uidmap' is 'uid loweruid count'\n"
          "  -g|--gmap gidmap : Group id mapping\n"
          "                     'gidmap' is 'gid lowergid count'\n"
          "  -s|--shell path  : Execute shell\n"
          "                     'path' is '%s' by default\n"
          ,
          basename(cmd),
          DEFAULT_CMD
         );
} // usage


int main(int ac, char *av[])
{
pid_t   child1, child2;
int     i, rc;
int     flags;
int     opt;
char   *shell = DEFAULT_CMD;
char  **umap;
int     numap;
char  **gmap;
int     ngmap;
int     sync1[2],  sync2[2];
int     status;

  umap = gmap = (char **)0;
  numap = ngmap = 0;
  
  // I don't want "getopt" to display messages when an option is invalid
  opterr = 0;

  // First argument to scan
  optind = 0;

  while ((opt = getopt_long(ac, av, ":hd:s:u:g:n:", shns_longopts, NULL)) != EOF) {

    switch (opt) {

      case 'h': {
        usage(av[0]);
        return 0;
      }
      break;

      case 'd': {
        if (!is_integer(optarg)) {
          fprintf(stderr, "Debug level must be an integer instead of '%s'\n", optarg);
          return 1;
	}
        dbg_level = atoi(optarg);
      }
      break;

      case 's': {
        shell = optarg;
      }
      break;

      case 'u': {
        rc = add_map(&umap, &numap, optarg);
        if (rc != 0) {
          ERR("add_map(%s): '%m' (%d)\n", optarg, errno);
          return 1;
	}
      }
      break;

      case 'g': {
        rc = add_map(&gmap, &ngmap, optarg);
        if (rc != 0) {
          ERR("add_map(%s): '%m' (%d)\n", optarg, errno);
          return 1;
	}
      }
      break;

      case 'n': { // namespace
        rc = select_ns(optarg);
        if (rc != 0) {
          ERR("Bad namespace '%s'\n", optarg);
          return 1;
        }
      }
      break;

      // Missing argument
      case ':' : {
        ERR("Missing value for '%c' option\n", optopt);
        usage(av[0]);
        return(1);
      }
      break;

      case '?' : {
        ERR("Bad option '%c'\n", optopt);
        usage(av[0]);
        return(1);
      }
      break;

      default: {
        usage(av[0]);
        return 1;
      }
      break;

    } // End switch

  } // End while

  // For each namespace of the target process, set the flag
  flags = 0;
  for (i = 0; i < NB_NS_NAME; i ++) {
    if (ns_list[i].selected) {
      DBG(1, "New namespace '%s'\n", ns_list[i].name);
      flags |= ns_list[i].flag;
    } // End if namespace selected
  } // End for

  // If no namespace passed, select all
  if (!flags) {
    for (i = 0; i < NB_NS_NAME; i ++) {
      ns_list[i].selected = 1;
      DBG(1, "New namespace '%s'\n", ns_list[i].name);
      flags |= ns_list[i].flag;
    } // End for
  }

  // If user namespace is requested, make sure that there are maps
  if (ns_list[SHNS_USER_IDX].selected && (!umap || !gmap)) {
    ERR("uid/gidmap must be specified along with user namespacce\n");
    return 1;
  }

  // If user namespace, make the maps
  if (ns_list[SHNS_USER_IDX].selected) {

    // Make the synchro pipe
    rc = pipe(sync1);
    if (0 != rc) {
      ERR("pipe(): '%m' (%d)\n", errno);
      return 1;
    }

    // We need to fork a process before changing the namespaces
    // to run new[ug]idmap otherwise the programs will check the
    // current user id in a user_ns where nothing is mapped (uid/gid = 65534).
    // So, the programs will end in error reporting that the current user
    // has no permission to make the mapping operation
    child1 = fork();

    switch(child1) {

      case 0: { // Child process#1

        char *cmdline;

        // Synchronize with father
        close(sync1[1]);
        rc = read(sync1[0], &child2, sizeof(child2));
        if (rc < 0) {
          ERR("read(): '%m' (%d)\n", errno);
          return 1;
        }
        close(sync1[0]);

        // Make the mappings

        cmdline = make_cmdline("newuidmap", child2, umap, numap);
        if (!cmdline) {
          ERR("make_cmdline(newuidmap): '%m' (%d)\n", errno);
          return 1;
        }

        DBG(1, "Running '%s'...\n", cmdline);
        rc = run_cmdline(cmdline);
        if (rc != 0) {
          ERR("run_cmdline(%s): '%m' (%d)\n", cmdline, errno);
          return 1;
        }

        free(cmdline);

        cmdline = make_cmdline("newgidmap", child2, gmap, ngmap);
        if (!cmdline) {
          ERR("make_cmdline(newgidmap): '%m' (%d)\n", errno);
          return 1;
        }

        DBG(1, "Running '%s'...\n", cmdline);
        rc = run_cmdline(cmdline);
        if (rc != 0) {
          ERR("run_cmdline(%s): '%m' (%d)\n", cmdline, errno);
          return 1;
        }

        free(cmdline);
        free_map(umap, numap);
        free_map(gmap, ngmap);

        // End of 1st child
        exit(0);

      }
      break;

      case -1 : { // Error

        ERR("fork(): '%m' (%d)\n", errno);
        return 1;

      }
      break;

      default: { // Father

        // Let's continue...

      }
      break;

    } // End switch

  } // End if user namespace

  // Make the synchro pipe
  rc = pipe(sync2);
  if (0 != rc) {
    ERR("pipe(): '%m' (%d)\n", errno);
    return 1;
  }

  // Create brand new namespaces
  rc = unshare(flags);
  if (rc != 0) {
    ERR("unshare(0x%x): '%m' (%d)\n", flags, errno);
    return 1;
  }

  // Fork a child process to make sure that it will be run
  // in new pid_ns (if any)
  child2 = fork();
  switch(child2) {

    case 0: { // Child process#2

      char *av_cmd[] = { shell, NULL };

      // Synchronize with father
      close(sync2[1]);
      rc = read(sync2[0], &i, sizeof(i));
      if (rc < 0) {
        ERR("read(): '%m' (%d)\n", errno);
        return 1;
      }
      close(sync2[0]);

      // Execute the shell
      extern char **environ;
      execve(av_cmd[0], av_cmd, environ);

      ERR("Unable to run '%s': %m (%d)\n", av_cmd[0], errno);
      _exit(1);

    }
    break;

    case -1 : { // Error

      ERR("fork(): '%m' (%d)\n", errno);
      return 1;

    }
    break;

    default: {

      // Father process, let's continue...
      close(sync2[0]);

    }
    break;

  } // End switch fork()

  // If user namespaces, synchronize with the first child
  if (ns_list[SHNS_USER_IDX].selected) {

    close(sync1[0]);
  
    // Synchronize with the 1st child
    rc = write(sync1[1], &child2, sizeof(child2));
    if (rc < 0) {
      ERR("write(): '%m' (%d)\n", errno);
      return 1;
    }

    // Wait for the end of the 1st child process
    rc = waitpid(child1, &status, 0);

    if (rc < 0) {
      ERR("waitpid(%d): %m (%d)", child1, errno);
      return 1;
    }

    // Some cleanup
    close(sync1[1]);

  } // End if user namespaces  

  // Synchronize with the 2nd child
  close(sync2[0]);
  i = 'y';
  rc = write(sync2[1], &i, sizeof(i));
  if (rc < 0) {
    ERR("write(): '%m' (%d)\n", errno);
    return 1;
  }

  // Wait for the end of the child process
  rc = waitpid(child2, &status, 0);

  if (rc < 0) {
    ERR("waitpid(%d): %m (%d)", child2, errno);
    return 1;
  }

  printf("program's status: %d (0x%x)\n", status, status);

  // Some cleanup
  close(sync2[1]);
  free_map(umap, numap);
  free_map(gmap, ngmap);

  return 0;  
} // main
