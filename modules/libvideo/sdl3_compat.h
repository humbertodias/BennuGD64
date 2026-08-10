/*
 * SDL3 helpers for BennuGD's software-surface video path.
 * SDL3 stores pixel format as an enum on SDL_Surface (no format pointer).
 */

#ifndef BENNU_SDL3_COMPAT_H
#define BENNU_SDL3_COMPAT_H

#include <SDL3/SDL.h>

static inline const SDL_PixelFormatDetails * bennu_surface_format( SDL_Surface * s )
{
    return s ? SDL_GetPixelFormatDetails( s->format ) : NULL;
}

static inline int bennu_surface_bpp( SDL_Surface * s )
{
    const SDL_PixelFormatDetails * f = bennu_surface_format( s );
    return f ? ( int ) f->bits_per_pixel : 0;
}

static inline int bennu_surface_bytes_pp( SDL_Surface * s )
{
    const SDL_PixelFormatDetails * f = bennu_surface_format( s );
    return f ? ( int ) f->bytes_per_pixel : 0;
}

static inline Uint32 bennu_surface_rmask( SDL_Surface * s )
{
    const SDL_PixelFormatDetails * f = bennu_surface_format( s );
    return f ? f->Rmask : 0;
}

static inline Uint32 bennu_surface_gmask( SDL_Surface * s )
{
    const SDL_PixelFormatDetails * f = bennu_surface_format( s );
    return f ? f->Gmask : 0;
}

static inline Uint32 bennu_surface_bmask( SDL_Surface * s )
{
    const SDL_PixelFormatDetails * f = bennu_surface_format( s );
    return f ? f->Bmask : 0;
}

static inline Uint32 bennu_surface_amask( SDL_Surface * s )
{
    const SDL_PixelFormatDetails * f = bennu_surface_format( s );
    return f ? f->Amask : 0;
}

static inline SDL_Palette * bennu_surface_palette( SDL_Surface * s )
{
    return s ? SDL_GetSurfacePalette( s ) : NULL;
}

static inline void bennu_set_surface_palette_colors( SDL_Surface * s, SDL_Color * colors, int first, int ncolors )
{
    SDL_Palette * pal;
    if ( !s ) return;
    pal = SDL_GetSurfacePalette( s );
    if ( !pal )
    {
        pal = SDL_CreateSurfacePalette( s );
        if ( !pal ) return;
    }
    SDL_SetPaletteColors( pal, colors, first, ncolors );
}

static inline SDL_Surface * bennu_create_rgb_surface( int w, int h, int depth,
                                                     Uint32 rmask, Uint32 gmask, Uint32 bmask, Uint32 amask )
{
    SDL_PixelFormat fmt = SDL_GetPixelFormatForMasks( depth, rmask, gmask, bmask, amask );
    return SDL_CreateSurface( w, h, fmt );
}

static inline SDL_Surface * bennu_create_rgb_surface_from( void * pixels, int w, int h, int depth, int pitch,
                                                          Uint32 rmask, Uint32 gmask, Uint32 bmask, Uint32 amask )
{
    SDL_PixelFormat fmt = SDL_GetPixelFormatForMasks( depth, rmask, gmask, bmask, amask );
    return SDL_CreateSurfaceFrom( w, h, fmt, pixels, pitch );
}

static inline Uint32 bennu_map_rgb( SDL_Surface * s, Uint8 r, Uint8 g, Uint8 b )
{
    return SDL_MapRGB( bennu_surface_format( s ), bennu_surface_palette( s ), r, g, b );
}

/* Hotkey callback still uses a small keysym-like view */
typedef struct Bennu_Keysym {
    SDL_Scancode scancode;
    SDL_Keycode sym;
    SDL_Keymod mod;
} Bennu_Keysym;

#endif /* BENNU_SDL3_COMPAT_H */
