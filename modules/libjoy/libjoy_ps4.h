#ifndef __LIBJOY_PS4_H
#define __LIBJOY_PS4_H

int         libjoy_ps4_num( void );
const char *libjoy_ps4_name( int joy );
int         libjoy_ps4_buttons( int joy );
int         libjoy_ps4_axes( int joy );
int         libjoy_ps4_hats( int joy );
int         libjoy_ps4_get_button( int joy, int button );
int         libjoy_ps4_get_position( int joy, int axis );
int         libjoy_ps4_get_hat( int joy, int hat );
int         libjoy_ps4_get_accel( int joy, int * x, int * y, int * z );
void        libjoy_ps4_module_initialize( void );
void        libjoy_ps4_module_finalize( void );
void        libjoy_ps4_pump( void );

#endif
