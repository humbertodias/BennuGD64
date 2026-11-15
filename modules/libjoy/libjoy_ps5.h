#ifndef __LIBJOY_PS5_H
#define __LIBJOY_PS5_H

int         libjoy_ps5_num( void );
const char *libjoy_ps5_name( int joy );
int         libjoy_ps5_buttons( int joy );
int         libjoy_ps5_axes( int joy );
int         libjoy_ps5_hats( int joy );
int         libjoy_ps5_get_button( int joy, int button );
int         libjoy_ps5_get_position( int joy, int axis );
int         libjoy_ps5_get_hat( int joy, int hat );
int         libjoy_ps5_get_accel( int joy, int * x, int * y, int * z );
void        libjoy_ps5_module_initialize( void );
void        libjoy_ps5_module_finalize( void );
void        libjoy_ps5_pump( void );

#endif
