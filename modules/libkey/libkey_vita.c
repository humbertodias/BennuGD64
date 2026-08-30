/*
 * PlayStation Vita keyboard init. Compiled only into the vita-arm build.
 */

#include "libkey_vita.h"

void libkey_vita_after_init( SDL_Window * window )
{
    ( void ) window;
    /* Do not call SDL_StartTextInput: it opens the IME at startup. */
}
