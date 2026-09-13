#ifndef __LIBJOY_XBOX360_H
#define __LIBJOY_XBOX360_H

int         libjoy_xbox360_num( void );
const char *libjoy_xbox360_name( int joy );
int         libjoy_xbox360_buttons( int joy );
int         libjoy_xbox360_axes( int joy );
int         libjoy_xbox360_hats( int joy );
int         libjoy_xbox360_get_button( int joy, int button );
int         libjoy_xbox360_get_position( int joy, int axis );
int         libjoy_xbox360_get_hat( int joy, int hat );
int         libjoy_xbox360_get_accel( int joy, int * x, int * y, int * z );
void        libjoy_xbox360_module_initialize( void );
void        libjoy_xbox360_module_finalize( void );
void        libjoy_xbox360_pump( void );

#endif
