#ifndef __DIRS_NATIVE_H
#define __DIRS_NATIVE_H

/*
 * Platform rmdir backend for dirs.c.
 * Default: dirs_native.c. Wii: dirs_wii.c (same symbols).
 */

int dir_native_rmdir( const char * path );

#endif
