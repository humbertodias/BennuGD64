/*
 * PlayStation 5 interpreter bootstrap. Compiled only into the ps5-x86_64 build.
 */

#ifndef __MAIN_PS5_H
#define __MAIN_PS5_H

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

char * bgdi_ps5_startup( int argc, char * argv[], int * standalone );

#endif
