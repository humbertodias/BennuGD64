#include <string.h>

#include <SDL3/SDL.h>
#include "sdl3_compat.h"
#include "libretro.h"
#include "libsdlhandler_libretro.h"

extern SDL_Window * window;
extern SDL_Surface * screen;

short int libretro_input_state_cb( unsigned port, unsigned device, unsigned index, unsigned id );

typedef struct {
    unsigned retro_id;
    SDL_Scancode scancode;
    SDL_Keycode keycode;
} libretro_key_map;

static const libretro_key_map key_map[] =
{
    { RETROK_a, SDL_SCANCODE_A, SDLK_A },
    { RETROK_b, SDL_SCANCODE_B, SDLK_B },
    { RETROK_c, SDL_SCANCODE_C, SDLK_C },
    { RETROK_d, SDL_SCANCODE_D, SDLK_D },
    { RETROK_e, SDL_SCANCODE_E, SDLK_E },
    { RETROK_f, SDL_SCANCODE_F, SDLK_F },
    { RETROK_g, SDL_SCANCODE_G, SDLK_G },
    { RETROK_h, SDL_SCANCODE_H, SDLK_H },
    { RETROK_i, SDL_SCANCODE_I, SDLK_I },
    { RETROK_j, SDL_SCANCODE_J, SDLK_J },
    { RETROK_k, SDL_SCANCODE_K, SDLK_K },
    { RETROK_l, SDL_SCANCODE_L, SDLK_L },
    { RETROK_m, SDL_SCANCODE_M, SDLK_M },
    { RETROK_n, SDL_SCANCODE_N, SDLK_N },
    { RETROK_o, SDL_SCANCODE_O, SDLK_O },
    { RETROK_p, SDL_SCANCODE_P, SDLK_P },
    { RETROK_q, SDL_SCANCODE_Q, SDLK_Q },
    { RETROK_r, SDL_SCANCODE_R, SDLK_R },
    { RETROK_s, SDL_SCANCODE_S, SDLK_S },
    { RETROK_t, SDL_SCANCODE_T, SDLK_T },
    { RETROK_u, SDL_SCANCODE_U, SDLK_U },
    { RETROK_v, SDL_SCANCODE_V, SDLK_V },
    { RETROK_w, SDL_SCANCODE_W, SDLK_W },
    { RETROK_x, SDL_SCANCODE_X, SDLK_X },
    { RETROK_y, SDL_SCANCODE_Y, SDLK_Y },
    { RETROK_z, SDL_SCANCODE_Z, SDLK_Z },
    { RETROK_0, SDL_SCANCODE_0, SDLK_0 },
    { RETROK_1, SDL_SCANCODE_1, SDLK_1 },
    { RETROK_2, SDL_SCANCODE_2, SDLK_2 },
    { RETROK_3, SDL_SCANCODE_3, SDLK_3 },
    { RETROK_4, SDL_SCANCODE_4, SDLK_4 },
    { RETROK_5, SDL_SCANCODE_5, SDLK_5 },
    { RETROK_6, SDL_SCANCODE_6, SDLK_6 },
    { RETROK_7, SDL_SCANCODE_7, SDLK_7 },
    { RETROK_8, SDL_SCANCODE_8, SDLK_8 },
    { RETROK_9, SDL_SCANCODE_9, SDLK_9 },
    { RETROK_SPACE, SDL_SCANCODE_SPACE, SDLK_SPACE },
    { RETROK_RETURN, SDL_SCANCODE_RETURN, SDLK_RETURN },
    { RETROK_ESCAPE, SDL_SCANCODE_ESCAPE, SDLK_ESCAPE },
    { RETROK_BACKSPACE, SDL_SCANCODE_BACKSPACE, SDLK_BACKSPACE },
    { RETROK_TAB, SDL_SCANCODE_TAB, SDLK_TAB },
    { RETROK_LEFT, SDL_SCANCODE_LEFT, SDLK_LEFT },
    { RETROK_RIGHT, SDL_SCANCODE_RIGHT, SDLK_RIGHT },
    { RETROK_UP, SDL_SCANCODE_UP, SDLK_UP },
    { RETROK_DOWN, SDL_SCANCODE_DOWN, SDLK_DOWN },
    { RETROK_LCTRL, SDL_SCANCODE_LCTRL, SDLK_LCTRL },
    { RETROK_RCTRL, SDL_SCANCODE_RCTRL, SDLK_RCTRL },
    { RETROK_LSHIFT, SDL_SCANCODE_LSHIFT, SDLK_LSHIFT },
    { RETROK_RSHIFT, SDL_SCANCODE_RSHIFT, SDLK_RSHIFT },
    { RETROK_LALT, SDL_SCANCODE_LALT, SDLK_LALT },
    { RETROK_RALT, SDL_SCANCODE_RALT, SDLK_RALT },
    { RETROK_LMETA, SDL_SCANCODE_LGUI, SDLK_LGUI },
    { RETROK_RMETA, SDL_SCANCODE_RGUI, SDLK_RGUI },
    { RETROK_BACKQUOTE, SDL_SCANCODE_GRAVE, SDLK_GRAVE },
    { RETROK_MINUS, SDL_SCANCODE_MINUS, SDLK_MINUS },
    { RETROK_EQUALS, SDL_SCANCODE_EQUALS, SDLK_EQUALS },
    { RETROK_LEFTBRACKET, SDL_SCANCODE_LEFTBRACKET, SDLK_LEFTBRACKET },
    { RETROK_RIGHTBRACKET, SDL_SCANCODE_RIGHTBRACKET, SDLK_RIGHTBRACKET },
    { RETROK_BACKSLASH, SDL_SCANCODE_BACKSLASH, SDLK_BACKSLASH },
    { RETROK_SEMICOLON, SDL_SCANCODE_SEMICOLON, SDLK_SEMICOLON },
    { RETROK_QUOTE, SDL_SCANCODE_APOSTROPHE, SDLK_APOSTROPHE },
    { RETROK_COMMA, SDL_SCANCODE_COMMA, SDLK_COMMA },
    { RETROK_PERIOD, SDL_SCANCODE_PERIOD, SDLK_PERIOD },
    { RETROK_SLASH, SDL_SCANCODE_SLASH, SDLK_SLASH },
    { RETROK_CAPSLOCK, SDL_SCANCODE_CAPSLOCK, SDLK_CAPSLOCK },
    { RETROK_NUMLOCK, SDL_SCANCODE_NUMLOCKCLEAR, SDLK_NUMLOCKCLEAR },
    { RETROK_SCROLLOCK, SDL_SCANCODE_SCROLLLOCK, SDLK_SCROLLLOCK },
    { RETROK_F1, SDL_SCANCODE_F1, SDLK_F1 },
    { RETROK_F2, SDL_SCANCODE_F2, SDLK_F2 },
    { RETROK_F3, SDL_SCANCODE_F3, SDLK_F3 },
    { RETROK_F4, SDL_SCANCODE_F4, SDLK_F4 },
    { RETROK_F5, SDL_SCANCODE_F5, SDLK_F5 },
    { RETROK_F6, SDL_SCANCODE_F6, SDLK_F6 },
    { RETROK_F7, SDL_SCANCODE_F7, SDLK_F7 },
    { RETROK_F8, SDL_SCANCODE_F8, SDLK_F8 },
    { RETROK_F9, SDL_SCANCODE_F9, SDLK_F9 },
    { RETROK_F10, SDL_SCANCODE_F10, SDLK_F10 },
    { RETROK_F11, SDL_SCANCODE_F11, SDLK_F11 },
    { RETROK_F12, SDL_SCANCODE_F12, SDLK_F12 },
    { RETROK_HOME, SDL_SCANCODE_HOME, SDLK_HOME },
    { RETROK_END, SDL_SCANCODE_END, SDLK_END },
    { RETROK_PAGEUP, SDL_SCANCODE_PAGEUP, SDLK_PAGEUP },
    { RETROK_PAGEDOWN, SDL_SCANCODE_PAGEDOWN, SDLK_PAGEDOWN },
    { RETROK_INSERT, SDL_SCANCODE_INSERT, SDLK_INSERT },
    { RETROK_DELETE, SDL_SCANCODE_DELETE, SDLK_DELETE },
    { RETROK_KP0, SDL_SCANCODE_KP_0, SDLK_KP_0 },
    { RETROK_KP1, SDL_SCANCODE_KP_1, SDLK_KP_1 },
    { RETROK_KP2, SDL_SCANCODE_KP_2, SDLK_KP_2 },
    { RETROK_KP3, SDL_SCANCODE_KP_3, SDLK_KP_3 },
    { RETROK_KP4, SDL_SCANCODE_KP_4, SDLK_KP_4 },
    { RETROK_KP5, SDL_SCANCODE_KP_5, SDLK_KP_5 },
    { RETROK_KP6, SDL_SCANCODE_KP_6, SDLK_KP_6 },
    { RETROK_KP7, SDL_SCANCODE_KP_7, SDLK_KP_7 },
    { RETROK_KP8, SDL_SCANCODE_KP_8, SDLK_KP_8 },
    { RETROK_KP9, SDL_SCANCODE_KP_9, SDLK_KP_9 },
    { RETROK_KP_PERIOD, SDL_SCANCODE_KP_PERIOD, SDLK_KP_PERIOD },
    { RETROK_KP_DIVIDE, SDL_SCANCODE_KP_DIVIDE, SDLK_KP_DIVIDE },
    { RETROK_KP_MULTIPLY, SDL_SCANCODE_KP_MULTIPLY, SDLK_KP_MULTIPLY },
    { RETROK_KP_MINUS, SDL_SCANCODE_KP_MINUS, SDLK_KP_MINUS },
    { RETROK_KP_PLUS, SDL_SCANCODE_KP_PLUS, SDLK_KP_PLUS },
    { RETROK_KP_ENTER, SDL_SCANCODE_KP_ENTER, SDLK_KP_ENTER },
    { RETROK_PAUSE, SDL_SCANCODE_PAUSE, SDLK_PAUSE },
    { RETROK_PRINT, SDL_SCANCODE_PRINTSCREEN, SDLK_PRINTSCREEN },
    { RETROK_MENU, SDL_SCANCODE_MENU, SDLK_MENU },
};

#define LIBRETRO_KEY_COUNT ( sizeof( key_map ) / sizeof( key_map[0] ) )

static uint8_t key_down[ LIBRETRO_KEY_COUNT ];
static uint8_t cb_down[ LIBRETRO_KEY_COUNT ];
static int cb_seen;
static uint16_t cb_mod;
/* key() reads SDL_GetKeyboardState(). Pushed events never update that
 * array under the dummy video driver, so Bennu reads this one instead. */
static bool key_held[ SDL_SCANCODE_COUNT ];
static float mouse_x;
static float mouse_y;
static uint8_t mouse_buttons;

static void push_key( const libretro_key_map * map, int down, SDL_Keymod mod )
{
    SDL_Event e;
    SDL_zero( e );
    e.type = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
    e.key.scancode = map->scancode;
    e.key.key = map->keycode;
    e.key.mod = mod;
    e.key.down = down ? true : false;
    e.key.repeat = false;
    e.key.windowID = window ? SDL_GetWindowID( window ) : 0;
    SDL_PushEvent( &e );
}

void libsdlhandler_libretro_keyboard_event( bool down, unsigned keycode, uint32_t character, uint16_t key_modifiers )
{
    unsigned i;
    ( void ) character;
    cb_seen = 1;
    cb_mod = key_modifiers;
    for ( i = 0; i < LIBRETRO_KEY_COUNT; i++ )
    {
        if ( key_map[i].retro_id != keycode )
            continue;
        cb_down[i] = down ? 1 : 0;
        return;
    }
}

const bool * libsdlhandler_libretro_keystate( void )
{
    return key_held;
}

static SDL_Keymod libretro_keymod( void )
{
    SDL_Keymod m = SDL_KMOD_NONE;

    if ( key_held[ SDL_SCANCODE_LSHIFT ] ) m |= SDL_KMOD_LSHIFT;
    if ( key_held[ SDL_SCANCODE_RSHIFT ] ) m |= SDL_KMOD_RSHIFT;
    if ( key_held[ SDL_SCANCODE_LCTRL ] ) m |= SDL_KMOD_LCTRL;
    if ( key_held[ SDL_SCANCODE_RCTRL ] ) m |= SDL_KMOD_RCTRL;
    if ( key_held[ SDL_SCANCODE_LALT ] ) m |= SDL_KMOD_LALT;
    if ( key_held[ SDL_SCANCODE_RALT ] ) m |= SDL_KMOD_RALT;
    if ( key_held[ SDL_SCANCODE_LGUI ] ) m |= SDL_KMOD_LGUI;
    if ( key_held[ SDL_SCANCODE_RGUI ] ) m |= SDL_KMOD_RGUI;

    if ( !cb_seen )
        return m;

    if ( cb_mod & RETROKMOD_CAPSLOCK ) m |= SDL_KMOD_CAPS;
    if ( cb_mod & RETROKMOD_NUMLOCK ) m |= SDL_KMOD_NUM;
    if ( cb_mod & RETROKMOD_SCROLLOCK ) m |= SDL_KMOD_SCROLL;
    if ( ( cb_mod & RETROKMOD_SHIFT ) && !( m & SDL_KMOD_SHIFT ) )
    {
        key_held[ SDL_SCANCODE_LSHIFT ] = true;
        m |= SDL_KMOD_LSHIFT;
    }
    if ( ( cb_mod & RETROKMOD_CTRL ) && !( m & SDL_KMOD_CTRL ) )
    {
        key_held[ SDL_SCANCODE_LCTRL ] = true;
        m |= SDL_KMOD_LCTRL;
    }
    if ( ( cb_mod & RETROKMOD_ALT ) && !( m & SDL_KMOD_ALT ) )
    {
        key_held[ SDL_SCANCODE_LALT ] = true;
        m |= SDL_KMOD_LALT;
    }
    return m;
}

void libsdlhandler_libretro_pump( void )
{
    unsigned i;
    int16_t dx, dy;
    uint8_t buttons = 0;
    SDL_Event e;

    while ( SDL_PeepEvents( &e, 1, SDL_GETEVENT, SDL_EVENT_FIRST, SDL_EVENT_LAST ) > 0 )
        ;

    memset( key_held, 0, sizeof( key_held ) );
    for ( i = 0; i < LIBRETRO_KEY_COUNT; i++ )
    {
        int down = libretro_input_state_cb( 0, RETRO_DEVICE_KEYBOARD, 0, key_map[i].retro_id ) ? 1 : 0;
        if ( cb_seen && cb_down[i] )
            down = 1;
        if ( down && key_map[i].scancode < SDL_SCANCODE_COUNT )
            key_held[ key_map[i].scancode ] = true;
        key_down[i] = ( uint8_t ) down;
    }
    {
        SDL_Keymod mod = libretro_keymod();
        static uint8_t sent[ LIBRETRO_KEY_COUNT ];
        SDL_SetModState( mod );
        for ( i = 0; i < LIBRETRO_KEY_COUNT; i++ )
        {
            if ( key_down[i] == sent[i] )
                continue;
            sent[i] = key_down[i];
            push_key( &key_map[i], key_down[i], mod );
        }
    }

    dx = libretro_input_state_cb( 0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_X );
    dy = libretro_input_state_cb( 0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_Y );
    if ( dx || dy )
    {
        float max_w = screen ? ( float ) screen->w : 320.0f;
        float max_h = screen ? ( float ) screen->h : 200.0f;
        mouse_x += ( float ) dx;
        mouse_y += ( float ) dy;
        if ( mouse_x < 0 ) mouse_x = 0;
        if ( mouse_y < 0 ) mouse_y = 0;
        if ( mouse_x >= max_w ) mouse_x = max_w - 1;
        if ( mouse_y >= max_h ) mouse_y = max_h - 1;
        SDL_zero( e );
        e.type = SDL_EVENT_MOUSE_MOTION;
        e.motion.x = mouse_x;
        e.motion.y = mouse_y;
        e.motion.xrel = ( float ) dx;
        e.motion.yrel = ( float ) dy;
        e.motion.windowID = window ? SDL_GetWindowID( window ) : 0;
        SDL_PushEvent( &e );
    }

    if ( libretro_input_state_cb( 0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_LEFT ) ) buttons |= 1;
    if ( libretro_input_state_cb( 0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_RIGHT ) ) buttons |= 2;
    if ( libretro_input_state_cb( 0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_MIDDLE ) ) buttons |= 4;

    {
        unsigned bit;
        unsigned ids[3] = { RETRO_DEVICE_ID_MOUSE_LEFT, RETRO_DEVICE_ID_MOUSE_RIGHT, RETRO_DEVICE_ID_MOUSE_MIDDLE };
        Uint8 sdlbtn[3] = { SDL_BUTTON_LEFT, SDL_BUTTON_RIGHT, SDL_BUTTON_MIDDLE };
        for ( bit = 0; bit < 3; bit++ )
        {
            uint8_t mask = ( uint8_t ) ( 1u << bit );
            if ( ( buttons & mask ) == ( mouse_buttons & mask ) )
                continue;
            SDL_zero( e );
            e.type = ( buttons & mask ) ? SDL_EVENT_MOUSE_BUTTON_DOWN : SDL_EVENT_MOUSE_BUTTON_UP;
            e.button.button = sdlbtn[bit];
            e.button.down = ( buttons & mask ) ? true : false;
            e.button.x = mouse_x;
            e.button.y = mouse_y;
            e.button.windowID = window ? SDL_GetWindowID( window ) : 0;
            SDL_PushEvent( &e );
            ( void ) ids;
        }
    }
    mouse_buttons = buttons;

    if ( libretro_input_state_cb( 0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_WHEELUP ) )
    {
        SDL_zero( e );
        e.type = SDL_EVENT_MOUSE_WHEEL;
        e.wheel.y = 1;
        SDL_PushEvent( &e );
    }
    if ( libretro_input_state_cb( 0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_WHEELDOWN ) )
    {
        SDL_zero( e );
        e.type = SDL_EVENT_MOUSE_WHEEL;
        e.wheel.y = -1;
        SDL_PushEvent( &e );
    }
}
