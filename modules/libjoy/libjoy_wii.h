#ifndef __LIBJOY_WII_H
#define __LIBJOY_WII_H

/* Native WPAD/PAD backend. SDL3-libogc2 OpenJoystick deadlocks Dolphin. */

void libjoy_wii_module_initialize( void );
void libjoy_wii_module_finalize( void );
void libjoy_wii_pump( void );

int         libjoy_wii_num( void );
const char *libjoy_wii_name( int joy );
int         libjoy_wii_buttons( int joy );
int         libjoy_wii_axes( int joy );
int         libjoy_wii_hats( int joy );
int         libjoy_wii_get_button( int joy, int button );
int         libjoy_wii_get_position( int joy, int axis );
int         libjoy_wii_get_hat( int joy, int hat );
int         libjoy_wii_get_accel( int joy, int * x, int * y, int * z );

#endif
