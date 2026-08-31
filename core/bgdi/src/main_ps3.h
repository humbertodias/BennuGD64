/*
 * PlayStation 3 interpreter bootstrap. Compiled only into the ps3-ppu build.
 * Include this from main.c so SDL_MAIN_HANDLED applies to the real main().
 */

#ifndef __MAIN_PS3_H
#define __MAIN_PS3_H

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

char * bgdi_ps3_startup( int argc, char * argv[], int * standalone );

#endif
