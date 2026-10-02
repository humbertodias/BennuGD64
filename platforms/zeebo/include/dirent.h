/* newlib's <dirent.h> is an #error stub. BREW has no POSIX directory API. */
#ifndef ZEEBO_DIRENT_H
#define ZEEBO_DIRENT_H

#define NAME_MAX 255

struct dirent
{
    char d_name[NAME_MAX + 1];
};

typedef struct DIR DIR;

#ifdef __cplusplus
extern "C" {
#endif

DIR * opendir( const char * name );
struct dirent * readdir( DIR * dirp );
int closedir( DIR * dirp );

#ifdef __cplusplus
}
#endif

#endif
