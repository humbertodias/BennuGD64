/*
 * Browser interpreter loop. Compiled only into the wasm build.
 */

#include <emscripten.h>

#include "bgdrtm.h"
#include "interpreter_p.h"

static void bgdrtm_browser_tick( void )
{
    /* Do not count RAFs: that assumed 60 Hz and made SET_FPS games (and
     * key() polling) run 2× on 120 Hz displays. */
    if ( !bgdrtm_browser_frame_due() )
        return;

    if ( !instance_go_one_turn() )
        emscripten_cancel_main_loop();
}

int instance_go_all()
{
    must_exit = 0 ;
    /* fps=0 → requestAnimationFrame. simulate_infinite_loop keeps main()
     * parked until the game exits (no SDL_Delay / emscripten_sleep). */
    emscripten_set_main_loop( bgdrtm_browser_tick, 0, 1 );
    return exit_value;
}
