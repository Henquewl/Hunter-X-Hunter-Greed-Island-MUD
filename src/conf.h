/* src/conf.h - Configured for Linux (Ubuntu 24.04 / glibc 2.39) */
#ifndef _CONF_H_
#define _CONF_H_

/* const keyword works fine */
/* #undef const */

/* vprintf is available */
#define HAVE_VPRINTF 1

/* sys/wait.h is POSIX.1 compatible */
#define HAVE_SYS_WAIT_H 1

/* Signal handlers return void on Linux */
#define RETSIGTYPE void

/* size_t is defined in sys/types.h */
/* #undef size_t */

/* ANSI C headers are available */
#define STDC_HEADERS 1

/* Can safely include both <sys/time.h> and <time.h> */
#define TIME_WITH_SYS_TIME 1

/* We're on Unix */
#define CIRCLE_UNIX 1

/* crypt() not used (no -lcrypt linkage needed on modern Linux) */
/* #undef CIRCLE_CRYPT */

/* Use our own crypt stub */
#define HAVE_UNSAFE_CRYPT 1

/* struct in_addr is available */
#define HAVE_STRUCT_IN_ADDR 1

/* socklen_t is defined in sys/socket.h — don't redefine */
/* #undef socklen_t */

/* ssize_t is defined in sys/types.h — don't redefine */
/* #undef ssize_t */

/* Functions available on Linux */
#define HAVE_GETTIMEOFDAY 1
#define HAVE_INET_ADDR 1
#define HAVE_INET_ATON 1
#define HAVE_SELECT 1
#define HAVE_SNPRINTF 1
#define HAVE_STRCASECMP 1
#define HAVE_STRDUP 1
#define HAVE_STRERROR 1
#define HAVE_STRLCPY 1
#define HAVE_STRNCASECMP 1
#define HAVE_STRSTR 1
#define HAVE_VSNPRINTF 1
/* stricmp / strnicmp are non-standard — not available on Linux */
/* #undef HAVE_STRICMP */
/* #undef HAVE_STRNICMP */

/* malloc library not needed */
/* #undef HAVE_LIBMALLOC */

/* Headers available on Linux */
#define HAVE_ARPA_INET_H 1
#define HAVE_ARPA_TELNET_H 1
#define HAVE_ASSERT_H 1
/* #undef HAVE_CRYPT_H */
#define HAVE_ERRNO_H 1
#define HAVE_FCNTL_H 1
#define HAVE_LIMITS_H 1
/* #undef HAVE_MCHECK_H */
#define HAVE_MEMORY_H 1
/* #undef HAVE_NET_ERRNO_H */
#define HAVE_NETDB_H 1
#define HAVE_NETINET_IN_H 1
#define HAVE_SIGNAL_H 1
#define HAVE_STRING_H 1
#define HAVE_STRINGS_H 1
/* #undef HAVE_SYS_FCNTL_H */
#define HAVE_SYS_RESOURCE_H 1
#define HAVE_SYS_SELECT_H 1
#define HAVE_SYS_SOCKET_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TIME_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_SYS_UIO_H 1
#define HAVE_UNISTD_H 1

/* All standard functions have proper prototypes on modern Linux —
   none of the NEED_*_PROTO defines are required */

#endif /* _CONF_H_ */
