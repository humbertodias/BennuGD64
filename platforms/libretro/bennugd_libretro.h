#ifndef BENNUGD_LIBRETRO_H
#define BENNUGD_LIBRETRO_H

#include <stddef.h>
#include <stdint.h>
#include "libretro.h"

int bgdi_main( int argc, char * argv[] );

void suspend_bgd( void );
void request_exit_bgd( void );

short int libretro_input_state_cb( unsigned port, unsigned device, unsigned index, unsigned id );

int  libretro_audio_sample_rate( void );
void libretro_audio_mix( void * mixbuf, size_t mixbuf_size );
int  modsound_libretro_start_output( void * mixer );

void libsdlhandler_libretro_pump( void );

extern int libretro_width;
extern int libretro_height;
extern int libretro_depth;
extern int libretro_scale_override;
extern int bennugd_content_width;
extern int bennugd_content_height;
extern bool retro_enable_frame_limiter;
extern retro_log_printf_t log_cb;

#endif
