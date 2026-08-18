/*
 * Emscripten runtime tweaks. Compiled only into the wasm build.
 */

#include "bgdrtm.h"
#include "offsets.h"
#include "misc_emscripten.h"

extern int libjoy_num( void );

void bgdrtm_emscripten_entry( void )
{
    /* SoRR: os_id 0 (Windows) → P1 keyboard; otherwise P1 is joy 0. */
    GLODWORD( OS_ID ) = libjoy_num() > 0 ? OS_LINUX : OS_WIN32;
}
