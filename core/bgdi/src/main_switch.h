/*
 * Nintendo Switch interpreter bootstrap. Compiled only into the switch-aarch64 build.
 * Include this from main.c so SDL_MAIN_HANDLED applies to the real main().
 */

#ifndef __MAIN_SWITCH_H
#define __MAIN_SWITCH_H

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

char * bgdi_switch_startup( int argc, char * argv[], int * standalone );

#endif
