#ifndef __G_VIDEO_XBOX360_H
#define __G_VIDEO_XBOX360_H

#include <SDL3/SDL.h>

void gr_video_xbox360_module_initialize( void );
void gr_video_xbox360_destroy( void );
void gr_video_xbox360_apply_mode( void );
int  gr_video_xbox360_present( SDL_Surface * src );
int  gr_video_xbox360_present_rects( SDL_Surface * src, const SDL_Rect * rects, int count );

#endif
