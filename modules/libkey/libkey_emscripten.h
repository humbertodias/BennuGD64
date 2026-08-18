#ifndef __LIBKEY_EMSCRIPTEN_H
#define __LIBKEY_EMSCRIPTEN_H

#include <SDL3/SDL.h>

void libkey_emscripten_after_init( SDL_Window * window );
int  libkey_emscripten_filter_keydown( const SDL_Event * e );

#endif
