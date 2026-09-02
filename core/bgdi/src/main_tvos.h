/*
 * Apple tvOS interpreter bootstrap. Compiled only into the tvos-arm64 build.
 * Include this from main.c. Include <SDL3/SDL_main.h> only in that TU
 * (the one with main); main_tvos.c must not include it or _main is duplicated.
 */

#ifndef __MAIN_TVOS_H
#define __MAIN_TVOS_H

#include <SDL3/SDL.h>

char * bgdi_tvos_startup( int argc, char * argv[], int * standalone );

#endif
