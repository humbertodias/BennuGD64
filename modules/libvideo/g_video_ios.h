#ifndef __G_VIDEO_IOS_H
#define __G_VIDEO_IOS_H

#include <SDL3/SDL.h>

void gr_video_ios_module_initialize( void );
void gr_video_ios_adjust_window( int * width, int * height, Uint32 * window_flags );
void gr_video_ios_apply_mode( void );
int  gr_video_ios_present( SDL_Surface * src );
int  gr_video_ios_present_rects( SDL_Surface * src, const SDL_Rect * rects, int count );

#endif
