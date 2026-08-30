/*
 * PlayStation Vita interpreter entry: clocks, heap, search path, default DCB.
 *
 * SoRR-vita always loaded a hardcoded data file and ignored extra argv from
 * the bubble / VitaShell. Do the same: only honor argv[1] when it looks like
 * a DCB. Prefer ux0:/data/bennugd64/main.dcb so a drop-in replaces the
 * bundled hello without rebuilding the VPK.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/stat.h>

#include <psp2/ctrl.h>
#include <psp2/power.h>

#include "main_vita.h"
#include "files.h"

/* vitasdk newlib default heap is too small for Bennu + SDL3. */
int _newlib_heap_size_user = 168 * 1024 * 1024;

static int vita_suffix_ok( const char * name, const char * ext )
{
    size_t n, e;
    if ( !name || !ext ) return 0;
    n = strlen( name );
    e = strlen( ext );
    if ( n < e ) return 0;
    return strcasecmp( name + n - e, ext ) == 0;
}

static int vita_arg_is_dcb( const char * a )
{
    return vita_suffix_ok( a, ".dcb" ) || vita_suffix_ok( a, ".dat" ) ||
           vita_suffix_ok( a, ".bin" );
}

/* LiveArea has no console. fprintf/exit look like a silent close.
 * vitasdk newlib has no dup2; freopen is enough for stdout/stderr. */
static void vita_redirect_stdio( void )
{
    mkdir( "ux0:/data", 0777 );
    mkdir( "ux0:/data/bennugd64", 0777 );
    if ( !freopen( "ux0:/data/bennugd64/bgdi.log", "w", stdout ) )
        return;
    /* "a" so the second open does not truncate the first. */
    if ( !freopen( "ux0:/data/bennugd64/bgdi.log", "a", stderr ) )
        return;
    setvbuf( stdout, NULL, _IONBF, 0 );
    setvbuf( stderr, NULL, _IONBF, 0 );
}

static int vita_dcb_exists( const char * path )
{
    FILE * test;

    test = fopen( path, "rb" );
    if ( !test )
        return 0;
    fclose( test );
    return 1;
}

/* LiveArea has no console. Same idea as ps2_missing_dcb(): keep a readable
 * screen up until the user quits. */
static void vita_missing_dcb( void )
{
    SDL_Window * window;
    SDL_Renderer * renderer;
    SDL_Event event;
    SceCtrlData pad;

    fprintf( stderr, "bgdi: main.dcb not found\n" );

    if ( !SDL_WasInit( SDL_INIT_VIDEO ) )
        SDL_Init( SDL_INIT_VIDEO | SDL_INIT_EVENTS );

    window = SDL_CreateWindow( "bgdi", 960, 544, SDL_WINDOW_FULLSCREEN );
    if ( !window )
        exit( 0 );
    renderer = SDL_CreateRenderer( window, NULL );
    if ( !renderer )
        exit( 0 );

    SDL_SetRenderScale( renderer, 2.0f, 2.0f );
    sceCtrlSetSamplingMode( SCE_CTRL_MODE_ANALOG );

    for ( ;; )
    {
        while ( SDL_PollEvent( &event ) )
        {
            if ( event.type == SDL_EVENT_QUIT )
                exit( 0 );
        }

        memset( &pad, 0, sizeof( pad ) );
        sceCtrlPeekBufferPositive( 0, &pad, 1 );
        if ( pad.buttons & ( SCE_CTRL_CROSS | SCE_CTRL_START | SCE_CTRL_SELECT ) )
            exit( 0 );

        SDL_SetRenderDrawColor( renderer, 16, 16, 48, 255 );
        SDL_RenderClear( renderer );
        SDL_SetRenderDrawColor( renderer, 255, 255, 255, 255 );
        SDL_RenderDebugText( renderer, 16, 24, "BennuGD64: main.dcb not found" );
        SDL_RenderDebugText( renderer, 16, 56, "Copy a game DCB to:" );
        SDL_RenderDebugText( renderer, 16, 72, "ux0:/data/bennugd64/main.dcb" );
        SDL_RenderDebugText( renderer, 16, 104, "The VPK also looks in:" );
        SDL_RenderDebugText( renderer, 16, 120, "app0:/main.dcb" );
        SDL_RenderDebugText( renderer, 16, 152, "Compile with this tree's" );
        SDL_RenderDebugText( renderer, 16, 168, "vita-host bgdc (not PC Bennu)." );
        SDL_RenderDebugText( renderer, 16, 200, "Log: ux0:/data/bennugd64/bgdi.log" );
        SDL_RenderDebugText( renderer, 16, 216, "CROSS / START / SELECT: quit" );
        SDL_RenderPresent( renderer );
        SDL_Delay( 16 );
    }
}

char * bgdi_vita_startup( int argc, char * argv[], int * standalone )
{
    static const char * bundled[] = {
        "ux0:/data/bennugd64/main.dcb",
        "app0:/main.dcb",
        "main.dcb",
        NULL
    };
    int k;

    scePowerSetArmClockFrequency( 444 );
    scePowerSetGpuClockFrequency( 222 );
    vita_redirect_stdio();
    fprintf( stderr, "bgdi: vita start argc=%d\n", argc );

    SDL_SetMainReady();
    SDL_SetHint( SDL_HINT_TOUCH_MOUSE_EVENTS, "0" );
#ifdef SDL_HINT_VITA_ENABLE_FRONT_TOUCH
    SDL_SetHint( SDL_HINT_VITA_ENABLE_FRONT_TOUCH, "0" );
#endif
#ifdef SDL_HINT_VITA_ENABLE_BACK_TOUCH
    SDL_SetHint( SDL_HINT_VITA_ENABLE_BACK_TOUCH, "0" );
#endif

    if ( !SDL_WasInit( SDL_INIT_VIDEO ) )
        SDL_Init( SDL_INIT_VIDEO | SDL_INIT_EVENTS );

    chdir( "app0:/" );
    file_addp( "ux0:/data/bennugd64/" );
    file_addp( "app0:/" );
    file_addp( "." );

    if ( standalone )
        *standalone = 1;

    if ( argc >= 2 && argv && argv[1] && argv[1][0] && vita_arg_is_dcb( argv[1] ) )
    {
        if ( vita_dcb_exists( argv[1] ) )
            return NULL;
        fprintf( stderr, "bgdi: missing argv %s\n", argv[1] );
        vita_missing_dcb();
    }

    for ( k = 0 ; bundled[k] ; k++ )
    {
        if ( vita_dcb_exists( bundled[k] ) )
        {
            fprintf( stderr, "bgdi: using %s\n", bundled[k] );
            if ( k == 0 )
                fprintf( stderr, "bgdi: ux0 main.dcb overrides the VPK; delete it to run app0:/main.dcb\n" );
            return ( char * ) bundled[k];
        }
        fprintf( stderr, "bgdi: missing %s\n", bundled[k] );
    }

    vita_missing_dcb();
    return NULL;
}
