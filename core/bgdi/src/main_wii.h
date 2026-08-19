/*
 * Nintendo Wii interpreter bootstrap. Compiled only into the wii-powerpc build.
 * Include this from main.c so SDL_MAIN_HANDLED applies to the real main().
 */

#ifndef __MAIN_WII_H
#define __MAIN_WII_H

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

char * bgdi_wii_startup( int argc, char * argv[], int * standalone );
void bgdi_wii_handle_power( void );

#endif
