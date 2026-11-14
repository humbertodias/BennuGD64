/*
 * Zeebo frame pacing. Compiled only into the zeebo-arm build.
 */

#include "g_frame_zeebo.h"

int gr_frame_zeebo_adjust_fps( int fps )
{
    /* SET_FPS(..., 0) means "no limit". On this console that runs the
     * intro in a flash and then falls over. 30 is the handset pace. */
    if ( fps <= 0 ) fps = 30;
    return fps;
}

int gr_frame_zeebo_adjust_skip( int skip )
{
    return skip;
}
