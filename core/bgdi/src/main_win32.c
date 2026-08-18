/*
 * Windows interpreter bits. Compiled only into the Win32 build.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _WIN32_WINNT 0x0500
#include <windows.h>

#include "bgdrtm.h"
#include "main_win32.h"

static void bgdi_win32_setup_console( void )
{
    SetConsoleOutputCP( CP_UTF8 );
    SetConsoleCP( CP_UTF8 );
}

char * bgdi_win32_resolve_argv0( int argc, char * argv[] )
{
    ( void ) argc;

    bgdi_win32_setup_console();

    if ( !argv || !argv[0] )
        return strdup( "bgdi.exe" );

    if ( strlen( argv[0] ) < 4 || strncmpi( &argv[0][strlen( argv[0] ) - 4], ".exe", 4 ) )
    {
        char * arg0 = malloc( strlen( argv[0] ) + 5 );
        sprintf( arg0, "%s.exe", argv[0] );
        return arg0;
    }

    return strdup( argv[0] );
}

int bgdi_win32_strip_exe_suffix( char * ext )
{
    if ( ext && !strncmpi( ext, ".exe", 4 ) )
    {
        *ext = '\0';
        return 1;
    }
    return 0;
}

void bgdi_win32_hide_own_console( void )
{
    HWND hWnd = GetConsoleWindow();
    DWORD dwProcessId;

    GetWindowThreadProcessId( hWnd, &dwProcessId );
    if ( dwProcessId == GetCurrentProcessId() ) ShowWindow( hWnd, SW_HIDE );
}
