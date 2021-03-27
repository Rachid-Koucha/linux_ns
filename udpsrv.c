/*

  Simple UDP server

  Author: R. Koucha
  Date: 25-Mar-2020

*/



#include <errno.h>
#include <sys/types.h>
#include <stdio.h>
#include <libgen.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "util.h"



static void usage(char *pg)
{
  fprintf(stderr, "Usage: %s -p port\n"
          ,
          basename(pg));

} // usage



int main(int ac, char *av[])
{
int   opt;
int   rc;
int   sd;
int   port;
struct sockaddr_in addr;
char buffer[128];


  // I don't want "getopt" to display messages when an option is invalid
  opterr = 0;

  // First argument to scan
  optind = 0;

  port = -1;

  while ((opt = getopt(ac, av, ":p:")) != EOF) {
    switch (opt) {
      case 'p': {
        port = atoi(optarg);
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

  if (port < 0) {
    usage(av[0]);
    return(1);
  }

  sd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sd < 0) {
    ERR("socket(): '%m' (%d)\n", errno);
    return 1;
  }

  // Address of loopback interface
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Loopback address

  rc = bind(sd, (struct sockaddr *)&addr, sizeof(addr));
  if (rc != 0) {
    ERR("bind(): '%m' (%d)\n", errno);
    return 1;
  }

  printf("Listening on port %d\n", port);

  rc = read(sd, buffer, sizeof(buffer) - 1);
  if (rc > 0) {
    buffer[rc] = '\0';
    printf("Received '%s'\n", buffer);
  }

  close(sd);

  return 0;
} // main



