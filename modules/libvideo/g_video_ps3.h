#ifndef __G_VIDEO_PS3_H
#define __G_VIDEO_PS3_H

#include <SDL3/SDL.h>

void gr_video_ps3_module_initialize( void );
void gr_video_ps3_destroy( void );
void gr_video_ps3_adjust_window( int * width, int * height, Uint32 * window_flags );
void gr_video_ps3_apply_mode( void );
int  gr_video_ps3_present( SDL_Surface * src );
int  gr_video_ps3_present_rects( SDL_Surface * src, const SDL_Rect * rects, int count );

#endif
