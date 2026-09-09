/* Shared OpenOrbis UserService and Pad lifecycle for PS4 input modules. */

#include <stdint.h>
#include <string.h>

#include <orbis/Pad.h>
#include <orbis/UserService.h>
#include <orbis/libkernel.h>

#include "ps4_platform.h"

static int ps4_pad_handle = -1;
static int ps4_platform_initialized;

int ps4_platform_initialize( void )
{
    OrbisUserServiceInitializeParams params;
    int32_t user = 0;
    int result;

    if ( ps4_platform_initialized )
        return ps4_pad_handle >= 0 ? 0 : -1;
    ps4_platform_initialized = 1;

    memset( &params, 0, sizeof( params ) );
    params.priority = ORBIS_KERNEL_PRIO_FIFO_LOWEST;
    sceUserServiceInitialize( &params );

    result = scePadInit();
    if ( result != 0 )
        return result;

    result = sceUserServiceGetInitialUser( &user );
    if ( result != 0 )
        return result;

    ps4_pad_handle = scePadOpen(
        user, ORBIS_PAD_PORT_TYPE_STANDARD, 0, NULL );
    return ps4_pad_handle >= 0 ? 0 : ps4_pad_handle;
}

int ps4_platform_pad_handle( void )
{
    return ps4_pad_handle;
}

int ps4_platform_read_pad( OrbisPadData * data )
{
    if ( ps4_pad_handle < 0 || !data )
        return -1;
    return scePadReadState( ps4_pad_handle, data );
}

int ps4_platform_get_ticks_ms( void )
{
    return ( int )( sceKernelGetProcessTime() / 1000 );
}

void ps4_platform_delay_ms( int delay )
{
    if ( delay > 0 )
        sceKernelUsleep( ( uint32_t ) delay * 1000u );
}
