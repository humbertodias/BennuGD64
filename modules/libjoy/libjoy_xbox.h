#ifndef __LIBJOY_XBOX_H
#define __LIBJOY_XBOX_H

void libjoy_xbox_pump( void );
void libjoy_xbox_module_initialize( void );
void libjoy_xbox_module_finalize( void );
int  libjoy_xbox_num( void );
const char * libjoy_xbox_name( int joy );
int  libjoy_xbox_buttons( int joy );
int  libjoy_xbox_axes( int joy );
int  libjoy_xbox_hats( int joy );
int  libjoy_xbox_get_button( int joy, int button );
int  libjoy_xbox_get_position( int joy, int axis );
int  libjoy_xbox_get_hat( int joy, int hat );
int  libjoy_xbox_get_accel( int joy, int * x, int * y, int * z );

#endif
