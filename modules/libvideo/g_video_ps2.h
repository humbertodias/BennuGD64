/*
 * PlayStation 2 video: gsKit via SDL3 renderer (640x448 NTSC default).
 */

#ifndef __G_VIDEO_PS2_H
#define __G_VIDEO_PS2_H

#include <SDL3/SDL.h>

void gr_video_ps2_module_initialize( void );
void gr_video_ps2_adjust_window( int * width, int * height, Uint32 * window_flags );
void gr_video_ps2_apply_mode( void );
void gr_video_ps2_before_window( void );
void gr_video_ps2_after_window( void );
int  gr_video_ps2_present( SDL_Surface * src );
int  gr_video_ps2_present_rects( SDL_Surface * src, const SDL_Rect * rects, int count );

#endif
