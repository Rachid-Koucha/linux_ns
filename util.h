


#ifndef UTIL_H
#define UTIL_H


#include <sys/types.h>
#include <stdio.h>


extern int dbg_level;

#define DBG(level, fmt, ...) do {		              \
    if (level <= dbg_level) {                                 \
      fprintf(stderr,                                         \
              "DEBUG_%d (%s#%d): "fmt,                        \
	      level, __FUNCTION__, __LINE__, ## __VA_ARGS__); \
    }                                                         \
  } while(0)


#define ERR(fmt, ...) do {                             \
    fprintf(stderr,                                    \
            "ERROR@%s#%d: "fmt,                      \
             __FUNCTION__, __LINE__, ## __VA_ARGS__);  \
                         } while(0)


extern int is_pid(const char *str);

extern int is_integer(const char *str);

extern int is_ns_name(const char *ns);

#define is_unsigned_integer(i) is_pid(i)


#define prompt(fmt, ...) do {                 \
                 printf(fmt, ## __VA_ARGS__); \
                 fflush(stdout);              \
               } while(0)

extern int getanswer(void);

extern int cmp_ns(pid_t pid1, pid_t pid2, const char *ns_name);


#endif // UTIL_H
