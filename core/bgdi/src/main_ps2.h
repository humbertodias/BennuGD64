/*
 * PlayStation 2 interpreter bootstrap. Compiled only into the ps2-mips build.
 * Include this from main.c so SDL_MAIN_HANDLED applies: argv is visible
 * before IOP reset. ISO and HostFS boot must not reset (CD / host: unmount).
 * USB boot must reset so usbd/usbhdfsd load.
 */

#ifndef __MAIN_PS2_H
#define __MAIN_PS2_H

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

char * bgdi_ps2_startup( int argc, char * argv[], int * standalone );
void   bgdi_ps2_use_device_argv0( const char * dcb, char ** argv );

#endif
