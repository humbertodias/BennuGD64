/*
 * Windows DirectDraw vsync helper. Compiled only into the Win32 build.
 */

#include <windows.h>
#include <initguid.h>
#include <ddraw.h>

#include "g_video_win32.h"

static LPDIRECTDRAW2 directdraw = NULL;
static DDCAPS ddcaps;

static HRESULT ( WINAPI * _DirectDrawCreate )( GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter );

static int init_dx( void )
{
    HINSTANCE handle;
    LPDIRECTDRAW directdraw1;
    HRESULT hr;

    handle = LoadLibrary( "DDRAW.DLL" );
    if ( handle == NULL ) return -1;

    _DirectDrawCreate = ( HRESULT ( WINAPI * )( GUID FAR *, LPDIRECTDRAW FAR *, IUnknown FAR * ) )
                        GetProcAddress( handle, "DirectDrawCreate" );
    if ( !_DirectDrawCreate ) return -1;

    hr = _DirectDrawCreate( NULL, &directdraw1, NULL );
    if ( FAILED( hr ) ) return -1;

    hr = IDirectDraw_QueryInterface( directdraw1, &IID_IDirectDraw2, ( LPVOID * ) &directdraw );
    if ( FAILED( hr ) ) return -1;

    IDirectDraw_Release( directdraw1 );

    hr = IDirectDraw2_SetCooperativeLevel( directdraw, NULL, DDSCL_NORMAL );
    if ( FAILED( hr ) ) return -1;

    ddcaps.dwSize = sizeof( ddcaps );
    hr = IDirectDraw2_GetCaps( directdraw, &ddcaps, NULL );
    if ( FAILED( hr ) ) return -1;

    return 0;
}

void gr_video_win32_module_initialize( void )
{
    if ( !directdraw ) init_dx();
}

void gr_video_win32_wait_vsync( void )
{
    if ( directdraw ) IDirectDraw2_WaitForVerticalBlank( directdraw, DDWAITVB_BLOCKBEGIN, NULL );
}

void gr_video_win32_module_finalize( void )
{
    if ( !directdraw ) return;

    IDirectDraw2_SetCooperativeLevel( directdraw, NULL, DDSCL_NORMAL );
    IDirectDraw2_Release( directdraw );
    directdraw = NULL;
}
