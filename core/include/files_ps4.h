#ifndef __FILES_PS4_H
#define __FILES_PS4_H

/* Directory that held the DCB (trailing slash). Relative fopen() is prefixed. */
void         file_ps4_bind_root( const char * dcb_path );
const char * file_ps4_root( void );

#endif
