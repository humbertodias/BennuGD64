#ifndef __G_VIDEO_XBOX_H
#define __G_VIDEO_XBOX_H

#include <SDL3/SDL.h>

void gr_video_xbox_module_initialize( void );
void gr_video_xbox_destroy( void );
void gr_video_xbox_apply_mode( void );
int  gr_video_xbox_present( SDL_Surface * src );
int  gr_video_xbox_present_rects( SDL_Surface * src, const SDL_Rect * rects, int count );

#endif
