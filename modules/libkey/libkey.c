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

#include <stdio.h>
#include <stdlib.h>

#include <SDL3/SDL.h>
#include "sdl3_compat.h"

#include "xctype.h"
#include "bgddl.h"
#include "dlvaracc.h"

#define __LIBKEY
#include "libkey.h"

#ifdef TARGET_PSP
#include "libkey_psp.h"
#endif
#ifdef TARGET_VITA
#include "libkey_vita.h"
#endif
#ifdef TARGET_EMSCRIPTEN
#include "libkey_emscripten.h"
#endif
#ifdef TARGET_WII
#include "libkey_wii.h"
#endif
#ifdef TARGET_PS2
#include "libkey_ps2.h"
#endif
#ifdef TARGET_PS3
#include "libkey_ps3.h"
#endif

/* ---------------------------------------------------------------------- */

#include "libkey_exports.h"

/* ---------------------------------------------------------------------- */

typedef struct
{
    SDL_Keycode sym;   /* SDL virtual keysym */
    SDL_Keymod mod;   /* current key modifiers */
    HOTKEY_CALLBACK callback;
} hotkey ;

static hotkey * hotkey_list = NULL ;
static int hotkey_allocated = 0 ;
static int hotkey_count = 0 ;

/* ---------------------------------------------------------------------- */

/* Publicas */
key_equiv key_table[127] ;              /* Now we have a search table with equivs */
const bool * keystate = NULL;        /* Pointer to key states */

extern SDL_Window * window ;

/* ---------------------------------------------------------------------- */

static int sdl_equiv[SDL_SCANCODE_COUNT] ;

static int equivs[] =
{
    SDL_SCANCODE_ESCAPE,        1,
    SDL_SCANCODE_1,             2,
    SDL_SCANCODE_KP_1,           2,
    SDL_SCANCODE_2,             3,
    SDL_SCANCODE_KP_2,           3,
    SDL_SCANCODE_3,             4,
    SDL_SCANCODE_KP_3,           4,
    SDL_SCANCODE_4,             5,
    SDL_SCANCODE_KP_4,           5,
    SDL_SCANCODE_5,             6,
    SDL_SCANCODE_KP_5,           6,
    SDL_SCANCODE_6,             7,
    SDL_SCANCODE_KP_6,           7,
    SDL_SCANCODE_7,             8,
    SDL_SCANCODE_KP_7,           8,
    SDL_SCANCODE_8,             9,
    SDL_SCANCODE_KP_8,           9,
    SDL_SCANCODE_9,             10,
    SDL_SCANCODE_KP_9,           10,
    SDL_SCANCODE_0,             11,
    SDL_SCANCODE_KP_0,           11,
    SDL_SCANCODE_MINUS,         12,
    SDL_SCANCODE_EQUALS,        13,
    SDL_SCANCODE_BACKSPACE,     14,
    SDL_SCANCODE_TAB,           15,
    SDL_SCANCODE_Q,             16,
    SDL_SCANCODE_W,             17,
    SDL_SCANCODE_E,             18,
    SDL_SCANCODE_R,             19,
    SDL_SCANCODE_T,             20,
    SDL_SCANCODE_Y,             21,
    SDL_SCANCODE_U,             22,
    SDL_SCANCODE_I,             23,
    SDL_SCANCODE_O,             24,
    SDL_SCANCODE_P,             25,
    SDL_SCANCODE_LEFTBRACKET,   26,
    SDL_SCANCODE_RIGHTBRACKET,  27,
    SDL_SCANCODE_RETURN,        28,
    SDL_SCANCODE_KP_ENTER,      28,
    SDL_SCANCODE_LCTRL,         29,
    SDL_SCANCODE_RCTRL,         29,
    SDL_SCANCODE_LCTRL,         96,
    SDL_SCANCODE_RCTRL,         94,
    SDL_SCANCODE_A,             30,
    SDL_SCANCODE_S,             31,
    SDL_SCANCODE_D,             32,
    SDL_SCANCODE_F,             33,
    SDL_SCANCODE_G,             34,
    SDL_SCANCODE_H,             35,
    SDL_SCANCODE_J,             36,
    SDL_SCANCODE_K,             37,
    SDL_SCANCODE_L,             38,
    SDL_SCANCODE_SEMICOLON,     39,
    SDL_SCANCODE_APOSTROPHE,         40,
    SDL_SCANCODE_GRAVE,     41,
    SDL_SCANCODE_LSHIFT,        42,
    SDL_SCANCODE_BACKSLASH,     43,
    SDL_SCANCODE_Z,             44,
    SDL_SCANCODE_X,             45,
    SDL_SCANCODE_C,             46,
    SDL_SCANCODE_V,             47,
    SDL_SCANCODE_B,             48,
    SDL_SCANCODE_N,             49,
    SDL_SCANCODE_M,             50,
    SDL_SCANCODE_COMMA,         51,
    SDL_SCANCODE_PERIOD,        52,
    SDL_SCANCODE_KP_PERIOD,     52,
    SDL_SCANCODE_SLASH,         53,
    SDL_SCANCODE_KP_DIVIDE,     53,
    SDL_SCANCODE_RSHIFT,        54,
    /*  SDLK_PRINT,         55,*/
    SDL_SCANCODE_KP_MULTIPLY,   55,
    SDL_SCANCODE_LALT,          56,
    SDL_SCANCODE_RALT,          56,
    SDL_SCANCODE_LALT,          95,
    SDL_SCANCODE_RALT,          93,
    SDL_SCANCODE_MODE,          93,
    SDL_SCANCODE_MODE,          56,
    SDL_SCANCODE_RGUI,         56,
    SDL_SCANCODE_LGUI,         56,
    SDL_SCANCODE_SPACE,         57,
    SDL_SCANCODE_CAPSLOCK,      58,
    SDL_SCANCODE_F1,            59,
    SDL_SCANCODE_F2,            60,
    SDL_SCANCODE_F3,            61,
    SDL_SCANCODE_F4,            62,
    SDL_SCANCODE_F5,            63,
    SDL_SCANCODE_F6,            64,
    SDL_SCANCODE_F7,            65,
    SDL_SCANCODE_F8,            66,
    SDL_SCANCODE_F9,            67,
    SDL_SCANCODE_F10,           68,
    SDL_SCANCODE_NUMLOCKCLEAR,       69,
    SDL_SCANCODE_SCROLLLOCK,     70,
    SDL_SCANCODE_HOME,          71,
    SDL_SCANCODE_UP,            72,
    SDL_SCANCODE_PAGEUP,        73,
    SDL_SCANCODE_KP_MINUS,      74,
    SDL_SCANCODE_LEFT,          75,
    SDL_SCANCODE_RIGHT,         77,
    SDL_SCANCODE_KP_PLUS,       78,
    SDL_SCANCODE_END,           79,
    SDL_SCANCODE_DOWN,          80,
    SDL_SCANCODE_PAGEDOWN,      81,
    SDL_SCANCODE_INSERT,        82,
    SDL_SCANCODE_DELETE,        83,
    SDL_SCANCODE_F11,           87,
    SDL_SCANCODE_F12,           88,
    SDL_SCANCODE_NONUSBACKSLASH,          89,
    SDL_SCANCODE_EQUALS,          90,
    SDL_SCANCODE_PERIOD,       91,
    SDL_SCANCODE_SLASH,      92,
    SDL_SCANCODE_6,         93,
    SDL_SCANCODE_SYSREQ,        55,
    SDL_SCANCODE_PAUSE,         95,
    SDL_SCANCODE_MENU,          97,
    SDL_SCANCODE_LGUI,        98,
    SDL_SCANCODE_RGUI,        99,
    SDL_SCANCODE_APPLICATION,       99,
    -1, -1
} ;

/* ---------------------------------------------------------------------- */

enum {
    SHIFTSTATUS = 0,
    ASCII,
    SCANCODE
};

/* ---------------------------------------------------------------------- */
/* Son las variables que se desea acceder.                           */
/* El interprete completa esta estructura, si la variable existe.    */
/* (usada en tiempo de ejecución)                                    */

DLVARFIXUP  __bgdexport( libkey, globals_fixup )[] =
{
    /* Nombre de variable global, puntero al dato, tamaño del elemento, cantidad de elementos */
    { "shift_status" , NULL, -1, -1 },
    { "ascii"        , NULL, -1, -1 },
    { "scan_code"    , NULL, -1, -1 },
    { NULL           , NULL, -1, -1 }
};

/* ---------------------------------------------------------------------- */

static void add_key_equiv( int equiv, int keyconst )
{
    key_equiv * curr = &key_table[keyconst] ;

    if ( curr->next != NULL ) while ( curr->next != NULL ) curr = curr->next ;

    if ( curr->sdlk_equiv != 0 )
    {
        curr->next = malloc( sizeof( key_equiv ) ) ;
        curr = curr->next ;
        curr->next = NULL ;
    }

    curr->sdlk_equiv = equiv ;
}

/* ---------------------------------------------------------------------- */

void hotkey_add( int mod, int sym, HOTKEY_CALLBACK callback )
{
    if ( hotkey_count >= hotkey_allocated )
    {
        hotkey_allocated = hotkey_count + 5;
        hotkey_list = realloc( hotkey_list, hotkey_allocated * sizeof( hotkey_list[0] ) );
    }

    if ( !hotkey_list )
    {
        fprintf( stderr, "No memory for alloc hotkey\n" );
        exit( -1 );
    }

    hotkey_list [hotkey_count].mod = mod;
    hotkey_list [hotkey_count].sym = sym;
    hotkey_list [hotkey_count].callback = callback;
    hotkey_count++;
}

/* ---------------------------------------------------------------------- */

/*
 *  FUNCTION : process_key_events
 *
 *  Process all pending SDL events, updating all global variables
 *  and handling debug key presses and cannot be changed
 *
 *  PARAMS :
 *      None
 *
 *  RETURN VALUE :
 *      None
 */

static void process_key_events()
{
    SDL_Event e ;
    SDL_Keymod m ;
    int k, asc ;
    int pressed ;
    key_equiv * curr ;
    int keypress ;
    static struct
    {
        int ascii ;
        int scancode ;
    }
    keyring [64] ;
    static int keyring_start = 0, keyring_tail = 0 ;
    int ignore_key, n;

#ifdef TARGET_PS2
    if ( !SDL_WasInit( SDL_INIT_VIDEO ) )
    {
        libkey_ps2_after_events();
        return;
    }
#endif

    /* Actualizar eventos */

    keypress = 0 ;
    m = SDL_GetModState() ;

    /* Procesa los eventos pendientes */
    /* Reset ascii and scancode if last key was released... */
    /* must check all the linked equivs */

    pressed = 0 ;
    if ( GLODWORD( libkey,  SCANCODE ) )
    {
        curr = &key_table[GLODWORD( libkey,  SCANCODE )] ;
        while ( curr != NULL && pressed == 0 )
        {
            if ( keystate[curr->sdlk_equiv] ) pressed = 1 ;
            curr = curr->next ;
        }
    }

    if ( !pressed )
    {
        GLODWORD( libkey,  ASCII )     = 0 ;
        GLODWORD( libkey,  SCANCODE )  = 0 ;
    }

    while ( SDL_PeepEvents( &e, 1, SDL_GETEVENT, SDL_KEYDOWN, SDL_KEYUP ) > 0 )
    {
        switch ( e.type )
        {
            case SDL_KEYDOWN:
#ifdef TARGET_EMSCRIPTEN
                if ( !libkey_emscripten_filter_keydown( &e ) )
                    break;
#endif
                ignore_key = 0;
                /* KeyDown HotKey */
                if ( hotkey_count )
                    for ( n = 0; n < hotkey_count; n++ )
                    {
                        if ((( hotkey_list[n].mod & e.key.mod ) == hotkey_list[n].mod ) &&
                                ( !hotkey_list[n].sym || ( hotkey_list[n].sym == e.key.key ) ) )
                        {
                            Bennu_Keysym ksym ;
                            ksym.scancode = e.key.scancode ;
                            ksym.sym = e.key.key ;
                            ksym.mod = e.key.mod ;
                            ignore_key = hotkey_list[n].callback( ksym );
                        }
                    }
                /* KeyDown HotKey */

                if ( ignore_key ) break ;

                /* Almacena la pulsación de la tecla */

                k = sdl_equiv[e.key.scancode];

                m = e.key.mod ;

                if ( e.key.key >= 32 && e.key.key < 256 )
                {
                    asc = win_to_dos[e.key.key & 0xFF] ;
                    if ( asc >= 'a' && asc <= 'z' && ( m & KMOD_LSHIFT || m & KMOD_RSHIFT || keystate[SDL_SCANCODE_CAPSLOCK] ) )
                        asc -= 0x20 ;
                }
                else
                {
                    asc = 0 ; /* NON PRINTABLE */
                }

                if ( !keypress )
                {
                    GLODWORD( libkey,  SCANCODE )  = k ;
                    GLODWORD( libkey,  ASCII ) = asc ;
                    keypress = 1 ;
                }
                else
                {
                    keyring[keyring_tail].scancode = k ;
                    keyring[keyring_tail].ascii = asc ;
                    if ( ++keyring_tail == 64 ) keyring_tail = 0 ;
                }

                break ;

            case SDL_KEYUP:
                /* Do nothing, Bennu is key_up unsensitive */
                break ;
        }
    }

    if ( !keypress && keyring_start != keyring_tail )
    {
        GLODWORD( libkey,  ASCII )     = keyring[keyring_start].ascii ;
        GLODWORD( libkey,  SCANCODE )  = keyring[keyring_start].scancode ;
        if ( ++keyring_start == 64 ) keyring_start = 0 ;
    }

    /* Now actualized every frame... */
    GLODWORD( libkey,  SHIFTSTATUS ) =
        ( ( m & KMOD_RSHIFT ) ? STAT_RSHIFT : 0 ) |
        ( ( m & KMOD_LSHIFT ) ? STAT_LSHIFT : 0 ) |

        ( ( m & KMOD_CTRL   ) ? STAT_CTRL   : 0 ) |
        ( ( m & KMOD_ALT    ) ? STAT_ALT    : 0 ) |

        ( ( m & KMOD_RCTRL  ) ? STAT_RCTRL  : 0 ) |
        ( ( m & KMOD_LCTRL  ) ? STAT_LCTRL  : 0 ) |

        ( ( m & KMOD_RALT   ) ? STAT_RALT   : 0 ) |
        ( ( m & KMOD_LALT   ) ? STAT_LALT   : 0 ) |

        ( ( m & KMOD_NUM    ) ? STAT_NUM    : 0 ) |
        ( ( m & KMOD_CAPS   ) ? STAT_CAPS   : 0 ) |
        ( ( m & KMOD_SHIFT  ) ? STAT_SHIFT  : 0 ) ;

#ifdef TARGET_WII
    libkey_wii_after_events();
#endif
#ifdef TARGET_VITA
    libkey_vita_after_events();
#endif
#ifdef TARGET_PS2
    libkey_ps2_after_events();
    if ( !GLODWORD( libkey, SCANCODE ) )
    {
        int ascii = 0;
        int sc = libkey_ps2_scan_code( &ascii );
        if ( sc )
        {
            GLODWORD( libkey, SCANCODE ) = sc;
            GLODWORD( libkey, ASCII ) = ascii;
        }
    }
    if ( keystate )
    {
        if ( keystate[ SDL_SCANCODE_LCTRL ] )
            GLODWORD( libkey, SHIFTSTATUS ) |= STAT_CTRL | STAT_LCTRL;
        if ( keystate[ SDL_SCANCODE_LALT ] )
            GLODWORD( libkey, SHIFTSTATUS ) |= STAT_ALT | STAT_LALT;
    }
#endif
#ifdef TARGET_PS3
    libkey_ps3_after_events();
#endif
}

/* ---------------------------------------------------------------------- */

/* Bigest priority first execute
   Lowest priority last execute */

HOOK __bgdexport( libkey, handler_hooks )[] =
{
    { 4900, process_key_events },
    {    0, NULL               }
} ;

/* ---------------------------------------------------------------------- */
/* Funciones de inicialización del módulo/plugin                     */

void __bgdexport( libkey, module_initialize )()
{
    int * ptr = equivs ;

#ifdef TARGET_PS2
    /* PADMAN lives in libkey_ps2. Skip SDL_Init VIDEO; SET_MODE owns gsKit. */
#else
    if ( !SDL_WasInit( SDL_INIT_VIDEO ) ) SDL_InitSubSystem( SDL_INIT_VIDEO );
#endif

    memset( sdl_equiv, 0, sizeof( sdl_equiv ) );
    memset( key_table, 0, sizeof( key_table ) ) ;

    while ( *ptr != -1 )
    {
        sdl_equiv[*ptr] = ptr[1] ;
        add_key_equiv( ptr[0], ptr[1] ) ;
        ptr += 2 ;
    }

    if ( !keystate ) keystate = SDL_GetKeyboardState( NULL );

#ifdef TARGET_PSP
    libkey_psp_after_init( window );
#elif defined(TARGET_VITA)
    libkey_vita_after_init( window );
#elif defined(TARGET_PS3)
    libkey_ps3_after_init( window );
#elif defined(TARGET_PS2)
    /* No OSK. DualShock is polled in libkey_ps2_after_events. */
#elif defined(TARGET_EMSCRIPTEN)
    libkey_emscripten_after_init( window );
#else
    if ( window ) SDL_StartTextInput( window );
#endif
}

/* ---------------------------------------------------------------------- */

void __bgdexport( libkey, module_finalize )()
{
    /* FREE used key_equivs... note base 127 equivs are static not allocated... */
    int i ;
    key_equiv * aux ;
    key_equiv * curr = key_table ;

    for ( i = 0;i < 127;i++ )
    {
        if ( curr->next != NULL )
        {
            curr = curr->next ;
            while ( curr->next != NULL )
            {
                aux = curr ;
                curr = curr->next;
                free( aux ) ;
            }
            free( curr ) ;
        }
    }

    if ( SDL_WasInit( SDL_INIT_VIDEO ) ) SDL_QuitSubSystem( SDL_INIT_VIDEO );
}

/* ---------------------------------------------------------------------- */
