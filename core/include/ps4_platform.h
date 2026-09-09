#ifndef __PS4_PLATFORM_H
#define __PS4_PLATFORM_H

#include <orbis/Pad.h>

int ps4_platform_initialize( void );
int ps4_platform_pad_handle( void );
int ps4_platform_read_pad( OrbisPadData * data );

#endif
