#ifndef LIBJOY_ZEEBO_H
#define LIBJOY_ZEEBO_H

void libjoy_zeebo_pump( void );
void libjoy_zeebo_module_initialize( void );
void libjoy_zeebo_module_finalize( void );
int libjoy_zeebo_num( void );
const char * libjoy_zeebo_name( int joy );
int libjoy_zeebo_buttons( int joy );
int libjoy_zeebo_axes( int joy );
int libjoy_zeebo_hats( int joy );
int libjoy_zeebo_get_button( int joy, int button );
int libjoy_zeebo_get_position( int joy, int axis );
int libjoy_zeebo_get_hat( int joy, int hat );
int libjoy_zeebo_get_accel( int joy, int * x, int * y, int * z );

#endif
