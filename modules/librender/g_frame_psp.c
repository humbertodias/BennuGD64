/*
 * PlayStation Portable frame pacing. Compiled only into the psp-mips build.
 */

#include "g_frame_psp.h"

int gr_frame_psp_adjust_skip( int skip )
{
    /* SoRR and similar titles call SET_FPS(..., 0). With skip 0 the engine
     * never drops a software blit, so logic stalls at draw speed. */
    if ( skip < 2 ) skip = 2;
    return skip;
}
