/* POSIX <dirent.h> on top of FindFirstFile for nxdk (no native dirent). */
#ifndef __BENNUGD_XBOX_DIRENT_H
#define __BENNUGD_XBOX_DIRENT_H

#include <windows.h>

#ifndef NAME_MAX
#define NAME_MAX 255
#endif

#ifndef DT_UNKNOWN
#define DT_UNKNOWN 0
#define DT_FIFO    1
#define DT_CHR     2
#define DT_DIR     4
#define DT_BLK     6
#define DT_REG     8
#define DT_LNK    10
#define DT_SOCK   12
#endif

struct dirent
{
  unsigned char d_type;
  char d_name[NAME_MAX + 1];
};

typedef struct DIR
{
  HANDLE handle;
  WIN32_FIND_DATA data;
  struct dirent ent;
  int first;
  int done;
} DIR;

#ifdef __cplusplus
extern "C" {
#endif

DIR *opendir (const char *name);
struct dirent *readdir (DIR *dirp);
int closedir (DIR *dirp);

#ifdef __cplusplus
}
#endif

#endif
