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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xctype.h"

/* Tabla de conversión de caracteres MS-DOS a Windows */

int dos_chars = 0 ;

unsigned char dos_to_win[256] = {
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
199, 252, 233, 226, 228, 224, 229, 231, 234, 235, 232, 239, 238, 236, 196, 197,
201, 230, 198, 244, 246, 242, 251, 249, 255, 214, 220, 248, 163, 216, 215, 131,
225, 237, 243, 250, 241, 209, 170, 186, 191, 174, 172, 189, 188, 161, 171, 187,
166, 166, 166, 166, 166, 193, 194, 192, 169, 166, 166,  43,  43, 162, 165,  43,
 43,  45,  45,  43,  45,  43, 227, 195,  43,  43,  45,  45, 166,  45,  43, 164,
240, 208, 202, 203, 200, 105, 205, 206, 207,  43,  43, 166,  95, 166, 204, 175,
211, 223, 212, 210, 245, 213, 181, 254, 222, 218, 219, 217, 253, 221, 175, 180,
173, 177,  61, 190, 182, 167, 247, 184, 176, 168, 183, 185, 179, 178, 166, 160
} ;

/* Tabla de conversión de caracteres Windows a MS-DOS */

unsigned char win_to_dos[256] =
{
   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
  96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
 128, 129, 130, 159, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
 255, 173, 189, 156, 207, 190, 254, 245, 249, 184, 166, 174, 170, 240, 169, 238,
 248, 241, 253, 252, 239, 230, 244, 250, 247, 251, 167, 175, 172, 171, 243, 168,
 183, 181, 182, 199, 142, 143, 146, 128, 212, 144, 210, 211, 222, 214, 215, 216,
 209, 165, 227, 224, 226, 229, 153, 158, 157, 235, 233, 234, 154, 237, 232, 225,
 133, 160, 131, 198, 132, 134, 145, 135, 138, 130, 136, 137, 141, 161, 140, 139,
 208, 164, 149, 162, 147, 228, 148, 246, 155, 151, 163, 150, 129, 236, 231, 152
} ;

/* Tipos de caracter */

char c_type[256] ;
unsigned char c_upper[256] ;
unsigned char c_lower[256] ;

int  c_type_initialized = 0 ;

unsigned char convert (unsigned char c)
{
    return dos_chars ? c : win_to_dos[c] ;
}

static void set_c_range (int first, int last, int type)
{
    while (first <= last) c_type[first++] |= type ;
}

static void set_c_from  (const unsigned char * chars, int type)
{
    if (dos_chars)
        while (*chars) c_type[win_to_dos[*chars++]] |= type ;
    else
        while (*chars) c_type[*chars++] |= type ;
}

static void set_c_as    (int prev_type, int type)
{
    int n ;

    for (n = 0 ; n < 256 ; n++)
    {
        if (c_type[n] & prev_type)
            c_type[n] |= type ;
    }
}

static void set_c_upper (const unsigned char * from, const unsigned char * to)
{
    if (dos_chars)
        while (*from) c_upper[win_to_dos[*from++]] = win_to_dos[*to++] ;
    else
        while (*from) c_upper[*from++] = *to++ ;
}

static void set_c_lower (const unsigned char * from, const unsigned char * to)
{
    if (dos_chars)
        while (*from) c_lower[win_to_dos[*from++]] = win_to_dos[*to++] ;
    else
        while (*from) c_lower[*from++] = *to++ ;
}

void init_c_type ()
{
    int c ;

    memset (c_type, 0, sizeof(c_type)) ;

    if (dos_chars)
        for (c = 0 ; c < 256 ; c++)
            c_lower[c] = c_upper[c] = win_to_dos[c] ;
    else
        for (c = 0 ; c < 256 ; c++)
            c_lower[c] = c_upper[c] = c ;

    set_c_upper  ((const unsigned char *)"abcdefghijklmnopqrstuvwxyz",
                  (const unsigned char *)"ABCDEFGHIJKLMNOPQRSTUVWXYZ") ;
    set_c_lower  ((const unsigned char *)"ABCDEFGHIJKLMNOPQRSTUVWXYZ",
                  (const unsigned char *)"abcdefghijklmnopqrstuvwxyz") ;
#ifdef RESPETAR_ACENTOS
    set_c_upper  ((const unsigned char *)"\xe1\xe9\xed\xf3\xfa\xf1\xe7\xe0\xe8\xec\xf2\xf9\xe4\xeb\xef\xf6\xfc\xfd\xe2\xea\xee\xf4\xfb\xe6\xe3\xe5\xf5",
                  (const unsigned char *)"\xc1\xc9\xcd\xd3\xda\xd1\xc7\xc0\xc8\xcc\xd2\xd9\xc4\xcb\xcf\xd6\xdc\xdd\xc2\xca\xce\xd4\xdb\xc6\xc3\xc5\xd5") ;
    set_c_lower  ((const unsigned char *)"\xc1\xc9\xcd\xd3\xda\xd1\xc7\xc0\xc8\xcc\xd2\xd9\xc4\xcb\xcf\xd6\xdc\xdd\xc2\xca\xce\xd4\xdb\xc6\xc3\xc5\xd5",
                  (const unsigned char *)"\xe1\xe9\xed\xf3\xfa\xf1\xe7\xe0\xe8\xec\xf2\xf9\xe4\xeb\xef\xf6\xfc\xfd\xe2\xea\xee\xf4\xfb\xe6\xe3\xe5\xf5") ;
#else
    set_c_upper  ((const unsigned char *)"\xe1\xe9\xed\xf3\xfa\xf1\xd1\xe7\xc7\xe0\xe8\xec\xf2\xf9\xe4\xeb\xef\xf6\xfc\xfd\xe2\xea\xee\xf4\xfb\xe6\xe3\xe5\xf5\xc1\xc9\xcd\xd3\xda",
                  (const unsigned char *)"AEIOUNNCCAEIOUAEIOUYAEIOU\xc6" "AAOAEIOU") ;
    set_c_lower  ((const unsigned char *)"\xe1\xe9\xed\xf3\xfa\xf1\xd1\xe7\xc7\xe0\xe8\xec\xf2\xf9\xe4\xeb\xef\xf6\xfc\xfd\xe2\xea\xee\xf4\xfb\xe6\xe3\xe5\xf5\xc1\xc9\xcd\xd3\xda",
                  (const unsigned char *)"aeiounnccaeiouaeiouyaeiou\xc6" "aaoaeiou") ;
#endif

    set_c_from  ((const unsigned char *)" \t\n\r", CTYPE_SPACE) ;

    set_c_range ('0', '9', CTYPE_NUM) ;

    set_c_range ('0', '9', CTYPE_ALNUM) ;
    set_c_range ('A', 'F', CTYPE_ALNUM) ;
    set_c_range ('a', 'f', CTYPE_ALNUM) ;

    set_c_range ('A', 'Z', CTYPE_ALPHA) ;
    set_c_range ('a', 'z', CTYPE_ALPHA) ;
    set_c_from  ((const unsigned char *)"\xe1\xe9\xed\xf3\xfa\xc1\xc9\xcd\xd3\xda\xf1\xd1\xe7\xc7\xe0\xe8\xec\xf2\xf9\xc0\xc8\xcc\xd2\xd9\xe4\xeb\xef\xf6\xfc\xc4\xcb\xcf\xd6\xdc\xff\xfd\xe2\xea\xee\xf4\xfb\xc2\xca\xce\xd4\xdb"
                 "\xe6\xc6\xe3\xe5\xf0\xf5\xaa\xba\xc3\xc5\xd0\xd5\xd8", CTYPE_ALPHA) ;

    set_c_as    (CTYPE_ALPHA, CTYPE_WORDCHAR) ;
    set_c_from  ((const unsigned char *)"_",         CTYPE_WORDCHAR) ;
    set_c_as    (CTYPE_WORDCHAR, CTYPE_WORDFIRST) ;

    set_c_range ('0', '9', CTYPE_WORDCHAR) ;

    c_type_initialized = 1 ;
}


