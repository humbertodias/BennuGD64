/* Minimal POSIX <sys/types.h> for nxdk/pdclib (no native sys/ tree). */
#ifndef __BENNUGD_XBOX_SYS_TYPES_H
#define __BENNUGD_XBOX_SYS_TYPES_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef long off_t;
typedef int pid_t;
typedef int ssize_t;
typedef unsigned int mode_t;
typedef unsigned int uid_t;
typedef unsigned int gid_t;
typedef unsigned int nlink_t;
typedef unsigned long ino_t;
typedef unsigned long dev_t;

#ifdef __cplusplus
}
#endif

#endif
