#ifndef __PS5_PLATFORM_H
#define __PS5_PLATFORM_H

#include "ps5_sce.h"

int ps5_platform_initialize( void );
int ps5_platform_pad_handle( void );
int ps5_platform_read_pad( ScePadData * data );
int ps5_platform_get_ticks_ms( void );
void ps5_platform_delay_ms( int delay );

#endif
