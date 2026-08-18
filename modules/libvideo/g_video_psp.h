/*
 * PlayStation Portable video backend. Compiled only into the psp-mips build.
 */

#ifndef __G_VIDEO_PSP_H
#define __G_VIDEO_PSP_H

#include <SDL3/SDL.h>

void gr_video_psp_module_initialize( void );
void gr_video_psp_destroy( void );
void gr_video_psp_adjust_window( int * width, int * height, Uint32 * window_flags );
void gr_video_psp_apply_mode( void );
int  gr_video_psp_ready_present( int logical_w, int logical_h );
int  gr_video_psp_present( SDL_Surface * src );
int  gr_video_psp_present_rects( SDL_Surface * src, const SDL_Rect * rects, int count );

#endif
