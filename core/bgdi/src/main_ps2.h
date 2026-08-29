/*
 * PlayStation 2 interpreter bootstrap. Compiled only into the ps2-mips build.
 * Include this from main.c so SDL_MAIN_HANDLED applies: argv is visible
 * before IOP reset. Probe host: and cdrom0: first (reset unmounts both), then
 * USB so File→Open + FAT32 .img still works when PCSX2 HostFS is enabled.
 */

#ifndef __MAIN_PS2_H
#define __MAIN_PS2_H

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

char * bgdi_ps2_startup( int argc, char * argv[], int * standalone );
void   bgdi_ps2_use_device_argv0( const char * dcb, char ** argv );

#endif
