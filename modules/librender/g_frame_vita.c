/*
 * PlayStation Vita frame pacing. Compiled only into the vita-arm build.
 */

#include "g_frame_vita.h"

int gr_frame_vita_adjust_skip( int skip )
{
    /* SoRR SET_FPS(..., 0) draws every FRAME. Skip 1 drops a blit when the
     * ARM is behind; Vita3K at 60 never takes this path. */
    if ( skip < 1 ) skip = 1;
    return skip;
}
