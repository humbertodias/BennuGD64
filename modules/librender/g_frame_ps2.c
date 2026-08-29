/*
 * PlayStation 2 frame pacing. Compiled only into the ps2-mips build.
 */

#include "g_frame_ps2.h"

int gr_frame_ps2_adjust_skip( int skip )
{
    /* SoRR SET_FPS(..., 0) would draw every FRAME; skip 2 drops a blit when
     * gsKit present is behind, without stacking sprites (floor wipe stays). */
    if ( skip < 2 ) skip = 2;
    return skip;
}
