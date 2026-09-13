/* Minimal POSIX <sys/stat.h> for nxdk (xboxrt exposes mode_t via <stat.h>). */
#ifndef __BENNUGD_XBOX_SYS_STAT_H
#define __BENNUGD_XBOX_SYS_STAT_H

#include <sys/types.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef S_IFMT
#define S_IFMT  0170000
#define S_IFDIR 0040000
#define S_IFCHR 0020000
#define S_IFBLK 0060000
#define S_IFREG 0100000
#define S_IFLNK 0120000
#define S_IFIFO 0010000
#endif

#ifndef S_ISREG
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m) (((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m) (((m) & S_IFMT) == S_IFBLK)
#endif

#ifndef _S_IFMT
#define _S_IFMT  S_IFMT
#define _S_IFREG S_IFREG
#define _S_IFDIR S_IFDIR
#endif

struct stat {
    dev_t st_dev;
    ino_t st_ino;
    mode_t st_mode;
    nlink_t st_nlink;
    uid_t st_uid;
    gid_t st_gid;
    dev_t st_rdev;
    off_t st_size;
    time_t st_atime;
    time_t st_mtime;
    time_t st_ctime;
};

int stat(const char *path, struct stat *buf);
int fstat(int fd, struct stat *buf);

#ifdef __cplusplus
}
#endif

#endif
