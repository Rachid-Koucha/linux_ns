/*

  Tool translating ids from one namespace to another

  Author: R. Koucha
  Date: 25-Mar-2020

*/


#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <stddef.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sched.h>
#include <fcntl.h>


#include "util.h"



// ----------------------------------------------------------------------------
// Name   : ID_MASK_XID
// Usage  : Requested ids
// ----------------------------------------------------------------------------
#define ID_MASK_PID 1
#define ID_MASK_UID 2
#define ID_MASK_GID 4


// ----------------------------------------------------------------------------
// Name   : open_socket
// Usage  : Open a socket with a namespace
// Return : the socket id, on success
//          -1, on failure (errno is set)
// ----------------------------------------------------------------------------
static int open_socket(
                       const char *sk_path,
                       int         srv
                      )
{
int                sd = -1;
size_t             len;
struct sockaddr_un addr;
int                rc;

  sd = socket(PF_UNIX, SOCK_STREAM, 0);
  if (sd < 0) {
    ERR("socket(%s): '%m' (%d)\n", sk_path, errno);
    rc = -1;
    goto err;
  }

  if (srv) {
    int sockVal;

    // Set some options on the socket
    sockVal = 1;
    if (0 != setsockopt(sd, SOL_SOCKET, SO_REUSEADDR,
	  	        (char *)&sockVal, sizeof (sockVal))) {
      ERR("setsockopt(%s, SO_REUSEADDR): '%m' (%d)\n", sk_path, errno);
      rc = -1;
      goto err;
    }

    memset(&addr, 0, sizeof(addr));

    addr.sun_family = AF_UNIX;
    rc = snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", sk_path);
    if (rc < 0 || rc >= (int)sizeof(addr.sun_path)) {
      if (rc >= 0) {
        errno = ENOSPC;
      }
      ERR("snprintf(%s): '%m' (%d)\n", sk_path, errno);
      rc = -1;
      goto err;
    }

    len = sizeof(struct sockaddr_un);

    rc = bind(sd, (struct sockaddr *)&addr, len);
    if (rc != 0) {
      ERR("bind(%s): '%m' (%d)\n", sk_path, errno);
      rc = -1;
      goto err;
    }

    // Update access rights on the socket file to make sure
    // that any user can connect to it
    (void)chmod(addr.sun_path, 0777);

    // Set the input connection queue length
    if (listen(sd, 5) == -1) {
      ERR("listen(%s): '%m' (%d)\n", sk_path, errno);
      rc = -1;
      goto err;
    }

  } else {

    // Client mode

    memset(&addr, 0, sizeof(addr));

    addr.sun_family = AF_UNIX;
    rc = snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", sk_path);
    if (rc < 0 || rc >= (int)sizeof(addr.sun_path)) {
      if (rc >= 0) {
        errno = ENOSPC;
      }
      ERR("snprintf(%s): '%m' (%d)\n", sk_path, errno);
      rc = -1;
      goto err;
    }

    len = sizeof(struct sockaddr_un);

    // Connect to the server
    rc = connect(sd, (const struct sockaddr *)&addr, len);
    if (0 != rc) {
      ERR("connect(%s): '%m' (%d)\n", sk_path, errno);
      rc = -1;
      goto err;
    }
  }

  rc = sd;

err:

  if (rc < 0) {

    if (sd >= 0) {
      close(sd);
    }
  }

  return rc;
} // open_socket


// -----------------------------------------------------------------------------
// Name   : recv_ids
// Usage  : Receive credentials
// Return : The translated pid, on success
//          -1, on failure (errno is set)
// -----------------------------------------------------------------------------
static int recv_ids(
                    int           sd,
                    struct ucred *ucreds
                         )
{
int             rc;
struct msghdr   msg;
struct cmsghdr *cmsg;
size_t          creds_sz = sizeof(struct ucred);
size_t          cmsg_buf_sz = CMSG_SPACE(creds_sz);
char           *cmsg_buf;
struct iovec    iov;
char            buf[1] = {0};

  memset(&msg, 0, sizeof(msg));

  cmsg_buf = (char *)malloc(cmsg_buf_sz);
  if (!cmsg_buf) {
    ERR("malloc(%zu): '%m' (%d)\n", cmsg_buf_sz, errno);
    rc = -1;
    goto err;
  }

  msg.msg_control = cmsg_buf;
  msg.msg_controllen = cmsg_buf_sz;

  iov.iov_base = buf;
  iov.iov_len = sizeof(buf);

  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;

  rc = recvmsg(sd, &msg, 0);
  if (-1 == rc) {
    ERR("recvmsg(%d): '%m' (%d)\n", sd, errno);
    rc = -1;
    goto err;
  }

  ucreds->pid = -1;
  ucreds->uid = -1;
  ucreds->gid = -1;

  cmsg = CMSG_FIRSTHDR(&msg);
  if (cmsg) {
    if (cmsg->cmsg_len == CMSG_LEN(creds_sz)) {
      if (SOL_SOCKET == cmsg->cmsg_level) {
        if (SCM_CREDENTIALS == cmsg->cmsg_type) {
          memcpy(ucreds, CMSG_DATA(cmsg), creds_sz);
	} else {
          errno = EINVAL;
          ERR("recvmsg(%d): '%m' (%d)\n", sd, errno);
          rc = -1;
          goto err;
	}
      } else {
        errno = EINVAL;
        ERR("recvmsg(%d): '%m' (%d)\n", sd, errno);
        rc = -1;
        goto err;
      }
    } else {
      errno = EINVAL;
      ERR("recvmsg(%d): '%m' (%d)\n", sd, errno);
      rc = -1;
      goto err;
    }
  } else {
    errno = EINVAL;
    ERR("recvmsg(%d): '%m' (%d)\n", sd, errno);
    rc = -1;
    goto err;
  }

  rc = 0;

err:

  if (cmsg_buf) {
    free(cmsg_buf);
  }

  return rc;

} // recv_ids


// -----------------------------------------------------------------------------
// Name   : send_ids
// Usage  : Send the pid in the credentials
// Return : 0, on success
//          -1, on failure (errno is set)
// -----------------------------------------------------------------------------
static int send_ids(
		    int   sd,
                    struct ucred *ucreds
                   )
{
int             rc;
struct msghdr   msg;
struct cmsghdr *cmsg;
size_t          creds_sz = sizeof(struct ucred);
size_t          cmsg_buf_sz = CMSG_SPACE(creds_sz);
char           *cmsg_buf;
struct iovec    iov;
char            buf[1] = {0};

  memset(&msg, 0, sizeof(msg));

  cmsg_buf = (char *)malloc(cmsg_buf_sz);
  if (!cmsg_buf) {
    ERR("malloc(%zu): '%m' (%d)\n", cmsg_buf_sz, errno);
    return -1;
  }

  msg.msg_control = cmsg_buf;
  msg.msg_controllen = cmsg_buf_sz;

  // According to man 7 unix, SCM_RIGHTS send or receive a set of open file
  // descriptors from another process.  The data portion contains an integer
  // array of the file descriptors. The passed file descriptors behave as
  // though they have been created with dup(2).
  cmsg = CMSG_FIRSTHDR(&msg);
  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type = SCM_CREDENTIALS;
  cmsg->cmsg_len = CMSG_LEN(creds_sz);
  memcpy(CMSG_DATA(cmsg), ucreds, creds_sz);

  // We need to attach some data otherwise nothing is
  // sent through the connection
  iov.iov_base = buf;
  iov.iov_len = sizeof(buf);
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;

  rc = sendmsg(sd, &msg, MSG_NOSIGNAL);
  if (-1 == rc) {
    ERR("sendmsg(%d): '%m' (%d)\n", sd, errno);
    free(cmsg_buf);
    return -1;
  }

  free(cmsg_buf);

  return 0;
} // send_ids




// -----------------------------------------------------------------------------
// Name   : child_getids
// Usage  : Child which runs in the target pid_ns to receive translated
//          process id from the father process and send the result back
// Return : 0, on success
//          1, on failure (errno is set)
// -----------------------------------------------------------------------------
static int child_getids(int srv_sd)
{
socklen_t             laddr;
int                   nfds;
fd_set                fdset;
struct sockaddr_un    addr;
int                   sd1 = -1;
int                   rc;

  // Wait for the connection on the server socket
  FD_ZERO(&fdset);
  FD_SET(srv_sd, &fdset);
  nfds = srv_sd + 1;
  rc = select(nfds, &fdset, 0, 0, 0);
  switch(rc) {

    // Error
    case -1: {
      ERR("select(%d): '%m' (%d)\n", srv_sd, errno);
      rc = 1;
      goto err;
    }
    break;

    // Connection request ?
    default: {

      if (FD_ISSET(srv_sd, &fdset)) {

        int          opt;
        struct ucred ucreds;

        // Accept the connection
        laddr = sizeof(addr);
        sd1 = accept(srv_sd, (struct sockaddr *)&addr, &laddr);
        if (sd1 < 0) {
          ERR("accept(%d): '%m' (%d)\n", srv_sd, errno);
          rc = 1;
          goto err;
        }

        // According to "man 7 unix", to receive a struct ucred message the
        // SO_PASSCRED option must be enabled on the socket
        opt = 1;
        rc = setsockopt(sd1, SOL_SOCKET, SO_PASSCRED, &opt, sizeof(opt));
        if (0 != rc) {
          // errno is set
          ERR("setsockopt(): '%m' (%d)\n", errno);
          rc = 1;
          goto err;
        }

        // Receive the credentials (with the translated target ids)
        rc = recv_ids(sd1, &ucreds);
        if (rc < 0) {
          // errno is set
          ERR("Unable to receive credentials: '%m' (%d)\n", errno);
          rc = 1;
          goto err;
        }

        // Send back the translated ids to the father in a data message
        rc = write(sd1, &ucreds, sizeof(ucreds));

        if (rc != (int)sizeof(ucreds)) {

          if (rc < 0) {
            ERR("write(%zu): '%m' (%d)\n", sizeof(ucreds), errno);
            rc = 1;
            goto err;
          }

          ERR("write(%zu): error\n", sizeof(ucreds));
          errno = EIO;
          rc = 1;
          goto err;
        }
      } // End if a bit in fdset
    }
    break;
  } // End switch

  rc = 0;

err:

  if (sd1 >= 0) {
    close(sd1);
  }

  if (srv_sd >= 0) {
    close(srv_sd);
  }

  // Exit status of the child
  return rc;

} // child_getids


// -----------------------------------------------------------------------------
// Name   : getnsids
// Usage  : Get the ids of a process into its pid_ns/user_ns
// Note   : man pid_namespaces
//
//                When a process ID is passed over a UNIX domain socket to a
//                process in a different PID namespace (see the description of
//                SCM_CREDENTIALS in unix(7)), it is translated into the
//                corresponding PID value in the receiving process's PID
//                namespace.
//
//          man user_namespaces
//
//                When a process's user and group IDs are passed over a UNIX
//                domain socket to a process in a different user namespace
//                (see the description of SCM_CREDENTIALS in unix(7)), they
//                are  translated into the corresponding values as per the
//                receiving process's user and group ID mappings.
//
// return : id in the target's pid_ns/user_ns, on success
//          -1, on failure (errno is set)
// -----------------------------------------------------------------------------
int getnsids(pid_t target, 
             unsigned int id_mask,
             pid_t *pid,
             uid_t *uid,
             gid_t *gid)
{
pid_t child = -1;
int   rc;
int   sd = -1, sd1 = -1;
char  sk_path[256];
int   status;
char  ns_path[256];
int   fd = -1;

  // Socket pathname
  rc = snprintf(sk_path, sizeof(sk_path), "/tmp/getnsids.%d", getpid());
  if (rc < 0 || rc >= (int)sizeof(sk_path)) {
    if (rc >= 0) {
      errno = ENOSPC;
    }
    ERR("snprintf(/tmp/getnsids.%d): '%m' (%d)\n", getpid(), errno);
    rc = 1;
    goto err;
  }

  // Remove the file if it exists (clean-up)
  unlink(sk_path);

  // Open the server socket
  sd = open_socket(sk_path, 1);
  if (sd < 0) {
    // errno is set
    return -1;
  }

  // pid_ns file of the target process
  if ((id_mask & ID_MASK_PID) &&
      !cmp_ns(getpid(), target, "pid")) {

    rc = snprintf(ns_path, sizeof(ns_path), "/proc/%d/ns/pid", target);
    if (rc < 0 || rc >= (int)sizeof(ns_path)) {
      if (rc >= 0) {
        errno = ENOSPC;
      }
      ERR("snprintf(/proc/%d/ns/pid): '%m' (%d)\n", target, errno);
      rc = 1;
      goto err;
    }

    // Open the pid_ns of the target process
    fd = open(ns_path, O_RDONLY);
    if (fd < 0) {
      ERR("open(%s): '%m' (%d)\n", ns_path, errno);
      rc = 1;
      goto err;
    }

    // Enter into the pid_ns of the target process
    rc = setns(fd, CLONE_NEWPID);
    if (rc < 0) {
      ERR("setns(NEWPID): '%m' (%d)\n", errno);
      rc = 1;
      goto err;
    }

    // No longer useful
    close(fd);
    fd = -1;

  } // End if pid translation is requested

  // User_ns of the target process
  if ((id_mask & (ID_MASK_UID | ID_MASK_GID)) &&
      !cmp_ns(getpid(), target, "user")) {

    rc = snprintf(ns_path, sizeof(ns_path), "/proc/%d/ns/user", target);
    if (rc < 0 || rc >= (int)sizeof(ns_path)) {
      if (rc >= 0) {
        errno = ENOSPC;
      }
      ERR("snprintf(/proc/%d/ns/user): '%m' (%d)\n", target, errno);
      rc = 1;
      goto err;
    }

    // Open the user_ns of the target process
    fd = open(ns_path, O_RDONLY);
    if (fd < 0) {
      ERR("open(%s): '%m' (%d)\n", ns_path, errno);
      rc = 1;
      goto err;
    }

    // Enter into the user_ns of the target process
    rc = setns(fd, CLONE_NEWUSER);
    if (rc < 0) {
      ERR("setns(NEWUSER): '%m' (%d)\n", errno);
      rc = 1;
      goto err;
    }

    // No longer useful
    close(fd);
    fd = -1;

  } // End if gid/uid translation is requested

  // Fork a process
  child = fork();

  switch(child) {

    // Child process
    case 0: {

      // The current process is in the target pid/user_ns
      rc = child_getids(sd);

      exit(rc);
    }
    break;

    // Fork error
    case -1: {

      ERR("fork(): '%m' (%d)\n", errno);
      rc = -1;
      goto err;

    }
    break;

    // Father process
    default: {

      struct ucred ucreds;

      // Close the server socket used by the child
      close(sd);
      sd = -1;

      // Connect to the server socket
      sd1 = open_socket(sk_path, 0);
      if (sd1 < 0) {
        rc = -1;
        goto err;
      }

      // Send the credentials with the ids
      ucreds.pid = (id_mask & ID_MASK_PID ? *pid : getpid());
      ucreds.uid = (id_mask & ID_MASK_UID ? *uid : getuid());
      ucreds.gid = (id_mask & ID_MASK_GID ? *gid : getgid());
      rc = send_ids(sd1, &ucreds);
      if (rc < 0) {
        ERR("Unable to send creds for pid#%d\n", target);
        rc = -1;
        goto err;
      }

      // Wait for the translated ids in the data of the message
      rc = read(sd1, &ucreds, sizeof(struct ucred));

      if (rc != (int)sizeof(struct ucred)) {

        if (rc >= 0) {
          errno = EIO;
	}

        if (rc < 0) {
          ERR("read(%d): '%m' (%d)\n", sd1, errno);
          rc = -1;
          goto err;
        }
      }

      // Close the socket
      close(sd1);
      sd1 = -1;

      *pid = ucreds.pid;
      *uid = ucreds.uid;
      *gid = ucreds.gid;

      // Wait for the end of the child process
      rc = waitpid(child, &status, 0);

      if (rc != child) {

        // For the error management at the end of the function
        child = -1;

        if (rc < 0) {
          ERR("waitpid(): '%m' (%d)\n", errno);
          rc = -1;
          goto err;
        }

        errno = EINVAL;
        ERR("waitpid(): Unexpected pid#%d\n", rc);
        rc = -1;
        goto err;
      }

      // For the error management at the end of the function
      child = -1;

      if (status != 0) {
        errno = EIO;
        ERR("status = 0x%x\n", status);
        rc = -1;
        goto err;
      }

    }
    break;

  } // End switch

  rc = 0;

err:

  // Remove the file if it exists
  unlink(sk_path);

  if (fd > 0) {
    close(fd);
  }

  if (sd1 >= 0) {
    close(sd1);
  }

  if (sd >= 0) {
    close(sd);
  }

  if (child >= 0) {
    // Wait for the end of the process
    waitpid(child, &status, 0);
  }

  return rc;
} // getnsids


static void usage(char *pg)
{
  fprintf(stderr, "Usage: %s -t pid [-p pid] [-u uid] [-g gid]\n", basename(pg));
} // usage


int main(int ac, char *av[])
{
  int   rc;
  int   opt;
  unsigned int id_mask = 0;
  pid_t target = -1;
  pid_t pid, tpid;
  uid_t uid, tuid;
  gid_t gid, tgid;

  // I don't want "getopt" to display messages when an option is invalid
  opterr = 0;

  // First argument to scan
  optind = 0;

  while ((opt = getopt(ac, av, ":t:p:u:g:")) != EOF) {

    switch (opt) {

      case 't': {  // Target process
        if (!is_pid(optarg)) {
          fprintf(stderr, "Option -t must be a pid instead of '%s'\n", optarg);
          return 1;
	}
        target = atoi(optarg);
      }
      break;
      case 'p': {  // Pid translation
        if (!is_pid(optarg)) {
          fprintf(stderr, "Option -p must be a pid instead of '%s'\n", optarg);
          return 1;
	}
        tpid = pid = atoi(optarg);
        id_mask |= ID_MASK_PID;
      }
      break;
      case 'u': {  // Uid translation
        tuid = uid = atoi(optarg);
        id_mask |= ID_MASK_UID;
      }
      break;
      case 'g': {  // Gid translation
        tgid = gid = atoi(optarg);
        id_mask |= ID_MASK_GID;
      }
      break;
      case ':' : { // Missing argument
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

  if (target < 0) {
    usage(av[0]);
    return(1);
  }

  if (!id_mask) {
    usage(av[0]);
    return(1);
  }

  // Translate the ids
  rc = getnsids(target, id_mask, &tpid, &tuid, &tgid);
  if (rc != -1) {

    // Display the translated ids
    if (id_mask & ID_MASK_PID) {
      printf("pid %d --> %d\n", pid, tpid);
    }

    if (id_mask & ID_MASK_UID) {
      printf("uid %d --> %d\n", uid, tuid);
    }

    if (id_mask & ID_MASK_GID) {
      printf("gid %d --> %d\n", gid, tgid);
    }
    return 0;
  } // End if ids translated

  return 1;
} // main
