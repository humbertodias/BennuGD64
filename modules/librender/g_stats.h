#ifndef __G_STATS_H
#define __G_STATS_H

#include "libgrbase.h"

void gr_stats_toggle( void );
void gr_stats_poll( void );
void gr_stats_draw( GRAPH * dest );
int  gr_stats_force_present( void );

#endif
