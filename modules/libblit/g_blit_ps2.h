#ifndef __G_BLIT_PS2_H
#define __G_BLIT_PS2_H

#include "libgrbase.h"

/* 1 if the blit was handled (empty GRAPH: wipe dest so sprites do not trail). */
int gr_blit_ps2_empty( GRAPH * dest, REGION * clip, GRAPH * gr );

#endif
