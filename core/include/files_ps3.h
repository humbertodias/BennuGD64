#ifndef __FILES_PS3_H
#define __FILES_PS3_H

/* Directory that held the DCB (trailing slash). Relative fopen() is prefixed. */
void         file_ps3_bind_root( const char * dcb_path );
const char * file_ps3_root( void );

#endif
