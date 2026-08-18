#ifndef __G_VIDEO_WII_H
#define __G_VIDEO_WII_H

#include <SDL3/SDL.h>

void gr_video_wii_module_initialize( void );
void gr_video_wii_adjust_window( int * width, int * height, Uint32 * window_flags );
void gr_video_wii_apply_mode( void );
void gr_video_wii_destroy( void );
int  gr_video_wii_present( SDL_Surface * src );
int  gr_video_wii_present_rects( SDL_Surface * src, const SDL_Rect * rects, int count );

#endif
