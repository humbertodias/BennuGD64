/*
 * PlayStation 2 blit. Compiled only into the ps2-mips build.
 *
 * Huge stage floors are GRAPH headers without pixels. A no-op blit leaves
 * the previous frame in the buffer, so run cycles stack until a full dump.
 * Wipe the dest clip instead so sprites land on a clean (black) floor.
 */

#include "g_blit_ps2.h"

int gr_blit_ps2_empty( GRAPH * dest, REGION * clip, GRAPH * gr )
{
    if ( gr && gr->data )
        return 0;
    if ( dest && dest->data )
        gr_clear_region( dest, clip );
    return 1;
}
