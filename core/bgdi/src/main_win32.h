#ifndef __MAIN_WIN32_H
#define __MAIN_WIN32_H

char * bgdi_win32_resolve_argv0( int argc, char * argv[] );
int   bgdi_win32_strip_exe_suffix( char * ext );
void  bgdi_win32_hide_own_console( void );

#endif
