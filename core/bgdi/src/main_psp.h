/*
 * PlayStation Portable interpreter bootstrap. Compiled only into the psp-mips build.
 * Include this from main.c so SDL_MAIN_HANDLED applies to the real main().
 */

#ifndef __MAIN_PSP_H
#define __MAIN_PSP_H

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

char * bgdi_psp_startup( int argc, char * argv[], int * standalone );

#endif
