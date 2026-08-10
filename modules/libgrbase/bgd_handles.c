/*
 * Opaque 32-bit handles for native pointers stored in BennuGD variables.
 */

#include <stdlib.h>
#include <string.h>

#include "bgd_handles.h"

static void ** handles = NULL;
static int handles_cap = 0;

int bgd_handle_put( void * ptr )
{
    int id;

    if ( !ptr ) return 0;

    for ( id = 1; id < handles_cap; id++ )
    {
        if ( !handles[id] )
        {
            handles[id] = ptr;
            return id;
        }
    }

    {
        int new_cap = handles_cap ? handles_cap * 2 : 256;
        void ** grown = ( void ** ) realloc( handles, ( size_t )new_cap * sizeof( void * ) );
        if ( !grown ) return 0;
        memset( grown + handles_cap, 0, ( size_t )( new_cap - handles_cap ) * sizeof( void * ) );
        handles = grown;
        id = handles_cap ? handles_cap : 1;
        handles_cap = new_cap;
        handles[id] = ptr;
        return id;
    }
}

void * bgd_handle_get( int handle )
{
    if ( handle <= 0 || handle >= handles_cap ) return NULL;
    return handles[handle];
}

void bgd_handle_free( int handle )
{
    if ( handle <= 0 || handle >= handles_cap ) return;
    handles[handle] = NULL;
}
