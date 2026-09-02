#ifndef __FILES_TVOS_H
#define __FILES_TVOS_H

/* Directory that held the DCB (trailing slash). Relative fopen() is prefixed. */
void         file_tvos_set_roots( const char * bundle, const char * docs );
void         file_tvos_bind_root( const char * dcb_path );
const char * file_tvos_root( void );

#endif
