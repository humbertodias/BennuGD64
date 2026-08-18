#ifndef __G_VIDEO_SWITCH_H
#define __G_VIDEO_SWITCH_H

#include <SDL3/SDL.h>

void gr_video_switch_module_initialize( void );
void gr_video_switch_adjust_window( int * width, int * height, Uint32 * window_flags );
void gr_video_switch_apply_mode( void );
int  gr_video_switch_present( SDL_Surface * src );
int  gr_video_switch_present_rects( SDL_Surface * src, const SDL_Rect * rects, int count );

#endif
