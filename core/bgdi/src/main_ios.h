/*
 * Apple iOS interpreter bootstrap. Compiled only into the ios-arm64 build.
 * Include this from main.c. Include <SDL3/SDL_main.h> only in that TU
 * (the one with main); main_ios.c must not include it or _main is duplicated.
 */

#ifndef __MAIN_IOS_H
#define __MAIN_IOS_H

#include <SDL3/SDL.h>

char * bgdi_ios_startup( int argc, char * argv[], int * standalone );

#endif
