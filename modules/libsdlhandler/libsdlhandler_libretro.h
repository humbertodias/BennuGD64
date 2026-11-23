#ifndef __LIBSDLHANDLER_LIBRETRO_H
#define __LIBSDLHANDLER_LIBRETRO_H

#include <stdbool.h>
#include <stdint.h>

void libsdlhandler_libretro_pump( void );
void libsdlhandler_libretro_keyboard_event( bool down, unsigned keycode, uint32_t character, uint16_t key_modifiers );
const bool * libsdlhandler_libretro_keystate( void );

#endif
