/*
 * PlayStation 2 interpreter bootstrap. Compiled only into the ps2-mips build.
 * Do not include SDL_main.h here: main.c includes it without SDL_MAIN_HANDLED
 * so SDL_RunApp() can reset IOP and mount host:/mass:/cdrom0:.
 */

#ifndef __MAIN_PS2_H
#define __MAIN_PS2_H

#include <SDL3/SDL.h>

char * bgdi_ps2_startup( int argc, char * argv[], int * standalone );

#endif
