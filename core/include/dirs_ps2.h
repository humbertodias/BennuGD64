#ifndef __DIRS_PS2_H
#define __DIRS_PS2_H

#include <stddef.h>

char * dirs_ps2_getcwd( char * buf, size_t size );
int    dirs_ps2_chdir( const char * dir );

#endif
