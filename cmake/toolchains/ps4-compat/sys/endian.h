/*
 * OpenOrbis ships musl <endian.h>, not FreeBSD <sys/endian.h>.
 * Clang --target=x86_64-pc-freebsd12-elf defines __FreeBSD__, so SDL3's
 * SDL_endian.h takes the FreeBSD branch and needs this wrapper.
 */
#ifndef _SYS_ENDIAN_H_
#define _SYS_ENDIAN_H_

#include <endian.h>

#endif /* _SYS_ENDIAN_H_ */
