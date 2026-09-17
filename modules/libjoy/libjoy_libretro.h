#ifndef __LIBJOY_LIBRETRO_H
#define __LIBJOY_LIBRETRO_H

void libjoy_libretro_module_initialize( void );
void libjoy_libretro_module_finalize( void );
int  libjoy_libretro_num( void );
const char * libjoy_libretro_name( int joy );
int  libjoy_libretro_buttons( int joy );
int  libjoy_libretro_axes( int joy );
int  libjoy_libretro_hats( int joy );
int  libjoy_libretro_get_button( int joy, int button );
int  libjoy_libretro_get_position( int joy, int axis );
int  libjoy_libretro_get_hat( int joy, int hat );
int  libjoy_libretro_get_accel( int joy, int * x, int * y, int * z );

#endif
