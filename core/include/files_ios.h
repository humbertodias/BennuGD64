#ifndef __FILES_IOS_H
#define __FILES_IOS_H

/* Directory that held the DCB (trailing slash). Relative fopen() is prefixed. */
void         file_ios_set_roots( const char * bundle, const char * docs );
void         file_ios_bind_root( const char * dcb_path );
const char * file_ios_root( void );

#endif
