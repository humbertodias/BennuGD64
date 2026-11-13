/* libretro-common often includes retro_common_api.h before any libc
   header. newlib's inttypes.h defines PRId64 only after sys/_stdint.h
   sets __int64_t_defined, which <sys/types.h> does. */
#ifndef BENNUGD_ZEEBO_INTTYPES_H
#define BENNUGD_ZEEBO_INTTYPES_H
#include <sys/types.h>
#endif
