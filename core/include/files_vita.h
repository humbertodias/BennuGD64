#ifndef __FILES_VITA_H
#define __FILES_VITA_H

/* Directory that held the DCB (trailing slash). Relative fopen() is prefixed. */
void         file_vita_bind_root( const char * dcb_path );
const char * file_vita_root( void );

#endif
