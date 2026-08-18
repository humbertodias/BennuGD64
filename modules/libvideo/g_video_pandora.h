#ifndef __G_VIDEO_PANDORA_H
#define __G_VIDEO_PANDORA_H

#include <SDL3/SDL.h>

void gr_video_pandora_module_initialize( void );
void gr_video_pandora_adjust_window( int * width, int * height, Uint32 * window_flags );
void gr_video_pandora_apply_mode( void );

#endif
