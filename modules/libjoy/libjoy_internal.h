#ifndef __LIBJOY_INTERNAL_H
#define __LIBJOY_INTERNAL_H

#include <SDL3/SDL.h>

#define MAX_JOYS    32

extern int _max_joys;
extern SDL_Joystick * _joysticks[MAX_JOYS];
extern SDL_JoystickID _joystick_ids[MAX_JOYS];
extern char _joystick_names[MAX_JOYS][128];

void libjoy_remember_name( int slot );

#endif
