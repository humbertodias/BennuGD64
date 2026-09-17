#include <SDL3/SDL.h>
#include "sdl3_compat.h"
#include "libretro.h"

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
};

static uint8_t key_down[ sizeof( key_map ) / sizeof( key_map[0] ) ];
static float mouse_x;
static float mouse_y;
static uint8_t mouse_buttons;

static void push_key( const libretro_key_map * map, int down )
{
    SDL_Event e;
    SDL_zero( e );
    e.type = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
    e.key.scancode = map->scancode;
    e.key.key = map->keycode;
    e.key.down = down ? true : false;
    e.key.repeat = false;
    e.key.windowID = window ? SDL_GetWindowID( window ) : 0;
    SDL_PushEvent( &e );
}

void libsdlhandler_libretro_pump( void )
{
    unsigned i;
    int16_t dx, dy;
    uint8_t buttons = 0;
    SDL_Event e;

    while ( SDL_PeepEvents( &e, 1, SDL_GETEVENT, SDL_EVENT_FIRST, SDL_EVENT_LAST ) > 0 )
        ;

    for ( i = 0; i < sizeof( key_map ) / sizeof( key_map[0] ); i++ )
    {
        int down = libretro_input_state_cb( 0, RETRO_DEVICE_KEYBOARD, 0, key_map[i].retro_id ) ? 1 : 0;
        if ( down != key_down[i] )
        {
            key_down[i] = ( uint8_t ) down;
            push_key( &key_map[i], down );
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
