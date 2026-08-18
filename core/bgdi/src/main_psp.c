/*
 * PlayStation Portable interpreter entry: clock, callbacks, search path, default DCB.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pspkernel.h>
#include <pspiofilemgr.h>
#include <pspthreadman.h>
#include <psppower.h>

#include "main_psp.h"
#include "files.h"
#include "dirs.h"

PSP_MODULE_INFO("BennuGD64", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(-2048);

int issetugid( void ) { return 0; }
char * getlogin( void ) { return NULL; }

static int psp_exit_callback ( int arg1, int arg2, void * common )
{
    ( void ) arg1;
    ( void ) arg2;
    ( void ) common;
    sceKernelExitGame();
    return 0;
}

static int psp_callback_thread ( SceSize args, void * argp )
{
    int cbid;

    ( void ) args;
    ( void ) argp;
    cbid = sceKernelCreateCallback( "Exit Callback", psp_exit_callback, NULL );
    sceKernelRegisterExitCallback( cbid );
    sceKernelSleepThreadCB();
    return 0;
}

static void psp_setup_callbacks ( void )
{
    int thid = sceKernelCreateThread( "update_thread", psp_callback_thread, 0x11, 0xFA0, 0, 0 );
    if ( thid >= 0 )
        sceKernelStartThread( thid, 0, 0 );
}

static void psp_bootstrap_paths ( int argc, char * argv[] )
{
    char cwd[ __MAX_PATH ];

    if ( getcwd( cwd, sizeof( cwd ) ) )
        file_addp( cwd );

    if ( argc > 0 && argv[0] && argv[0][0] )
    {
        char * copy = strdup( argv[0] );
        char * slash = strrchr( copy, '/' );
        if ( slash )
        {
            *slash = '\0';
            chdir( copy );
            file_addp( copy );
        }
        free( copy );
    }

    file_addp( "." );
}

char * bgdi_psp_startup( int argc, char * argv[], int * standalone )
{
    scePowerSetClockFrequency( 333, 333, 166 );
    psp_setup_callbacks();
    SDL_SetMainReady();
    if ( !SDL_WasInit( SDL_INIT_VIDEO ) )
        SDL_Init( SDL_INIT_VIDEO | SDL_INIT_EVENTS );
    psp_bootstrap_paths( argc, argv );

    if ( standalone )
        *standalone = 1;

    if ( argc >= 2 )
        return NULL;

    if ( access( "main.dcb", R_OK ) == 0 )
        return "main.dcb";

    return NULL;
}
