/* Shared UserService and Pad lifecycle for PS5 input modules. */

#include <string.h>

#include "ps5_platform.h"

static int ps5_pad_handle = -1;
static int ps5_platform_initialized;

int ps5_platform_initialize( void )
{
    int user = 0;
    int result;

    if ( ps5_platform_initialized )
        return ps5_pad_handle >= 0 ? 0 : -1;
    ps5_platform_initialized = 1;

    sceSystemServiceHideSplashScreen();
    sceUserServiceInitialize( NULL );

    result = scePadInit();
    if ( result != 0 )
        return result;

    result = sceUserServiceGetInitialUser( &user );
    if ( result != 0 )
        result = sceUserServiceGetForegroundUser( &user );
    if ( result != 0 )
        user = 0xff;

    ps5_pad_handle = scePadOpen( user, SCE_PAD_PORT_TYPE_STANDARD, 0, NULL );
    return ps5_pad_handle >= 0 ? 0 : ps5_pad_handle;
}

int ps5_platform_pad_handle( void )
{
    return ps5_pad_handle;
}

int ps5_platform_read_pad( ScePadData * data )
{
    if ( ps5_pad_handle < 0 || !data )
        return -1;
    return scePadReadState( ps5_pad_handle, data );
}

int ps5_platform_get_ticks_ms( void )
{
    return ( int )( sceKernelGetProcessTime() / 1000 );
}

void ps5_platform_delay_ms( int delay )
{
    if ( delay > 0 )
        sceKernelUsleep( ( unsigned int ) delay * 1000u );
}
