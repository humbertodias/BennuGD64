/*
 * Android interpreter bootstrap. Compiled only into the android build.
 */

#ifndef __MAIN_ANDROID_H
#define __MAIN_ANDROID_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

char * bgdi_android_startup( int argc, char * argv[], int * standalone );

#endif
