/*
 *  Copyright © 2006-2013 SplinterGU (Fenix/Bennugd)
 *  Copyright © 2002-2006 Fenix Team (Fenix)
 *  Copyright © 1999-2002 José Luis Cebrián Pagüe (Fenix)
 *
 *  This file is part of Bennu - Game Development
 *
 *  This software is provided 'as-is', without any express or implied
 *  warranty. In no event will the authors be held liable for any damages
 *  arising from the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *     1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 *
 *     2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 *
 *     3. This notice may not be removed or altered from any source
 *     distribution.
 *
 */

#ifndef __ARRANGE_ST_H
    #define __ARRANGE_ST_H

    #include <stdint.h>

    #define __LIL_ENDIAN 1234
    #define __BIG_ENDIAN 4321

    #if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        #define __BYTEORDER  __BIG_ENDIAN
    #elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        #define __BYTEORDER  __LIL_ENDIAN
    #elif defined(__hppa__) || \
        defined(__m68k__) || \
        defined(mc68000) || \
        defined(_M_M68K) || \
        (defined(__MIPS__) && defined(__MISPEB__)) || \
        defined(__powerpc__) || \
        defined(__ppc__) || \
        defined(__PPC__) || \
        defined(__POWERPC__) || \
        defined(_ARCH_PPC) || \
        defined(_M_PPC) || \
        defined(__sparc__)
        #define __BYTEORDER  __BIG_ENDIAN
    #else
        #define __BYTEORDER  __LIL_ENDIAN
    #endif

    /* ---------------------------------------------------------------------- */
    /* Trucos de portabilidad                                                 */
    /* ---------------------------------------------------------------------- */

    #if __BYTEORDER == __LIL_ENDIAN
        #define ARRANGE_DWORD(x)
        #define ARRANGE_WORD(x)

        #define ARRANGE_DWORDS(x,c)
        #define ARRANGE_WORDS(x,c)
    #else
        #include <string.h>

        static __inline__ void DO_Swap16(uint16_t * D) {
            uint16_t v;
            memcpy( &v, D, sizeof( v ) );
            v = ( uint16_t )( ( v << 8 ) | ( v >> 8 ) );
            memcpy( D, &v, sizeof( v ) );
        }

        static __inline__ void DO_Swap32(uint32_t * D) {
            uint32_t v;
            memcpy( &v, D, sizeof( v ) );
            v = ( ( v << 24 ) | ( ( v << 8 ) & 0x00FF0000 ) | ( ( v >> 8 ) & 0x0000FF00 ) | ( v >> 24 ) );
            memcpy( D, &v, sizeof( v ) );
        }

        #define ARRANGE_DWORD(x)    DO_Swap32(x)
        #define ARRANGE_WORD(x)     DO_Swap16(x)

        #define ARRANGE_DWORDS(x,c) {               \
            int __n;                                \
            uint32_t * __p = (uint32_t *)(x);       \
            for (__n = 0 ; __n < (int)(c) ; __n++)  \
                ARRANGE_DWORD(&__p[__n]);           \
            }
        #define ARRANGE_WORDS(x,c) {                \
            int __n;                                \
            uint16_t * __p = (uint16_t *)(x);       \
            for (__n = 0 ; __n < (int)(c) ; __n++)  \
                ARRANGE_WORD(&__p[__n]);            \
            }
    #endif
#endif
