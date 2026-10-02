#ifndef __G_VIDEO_ZEEBO_H
#define __G_VIDEO_ZEEBO_H

#include <SDL3/SDL.h>

void gr_video_zeebo_module_initialize( void );
void gr_video_zeebo_adjust_window( int * width, int * height, Uint32 * window_flags );
void gr_video_zeebo_apply_mode( void );
void gr_video_zeebo_present( SDL_Surface * src );

#endif
