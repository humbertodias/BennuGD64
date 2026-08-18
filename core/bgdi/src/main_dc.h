/*
 * Dreamcast interpreter bootstrap. Compiled only into the dreamcast-sh4 build.
 * Include this from main.c so SDL_MAIN_HANDLED applies to the real main().
 */

#ifndef __MAIN_DC_H
#define __MAIN_DC_H

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

char * bgdi_dc_startup( int argc, char * argv[], int * standalone );

#endif
