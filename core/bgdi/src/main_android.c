/*
 * Android interpreter entry: extract bundled DCBs into internal storage.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "main_android.h"
#include "files.h"
#include "dirs.h"

static int android_write_file ( const char * path, const void * data, size_t size )
{
    FILE * fp = fopen( path, "wb" );
    if ( !fp ) return 0;
    if ( size && fwrite( data, 1, size, fp ) != size )
    {
        fclose( fp );
        return 0;
    }
    fclose( fp );
    return 1;
}

static void android_extract_assets ( const char * dest_dir )
{
    const char * bundled[] = { "main.dcb", "hello.dcb", NULL };
    int i;

    for ( i = 0 ; bundled[i] ; i++ )
    {
        char dest[ __MAX_PATH ];
        size_t sz = 0;
        void * data;

        snprintf( dest, sizeof( dest ), "%s/%s", dest_dir, bundled[i] );
        if ( access( dest, R_OK ) == 0 ) continue;
        data = SDL_LoadFile( bundled[i], &sz );
        if ( data )
        {
            android_write_file( dest, data, sz );
            SDL_free( data );
        }
    }
}

char * bgdi_android_startup( int argc, char * argv[], int * standalone )
{
    const char * storage = SDL_GetAndroidInternalStoragePath();

    ( void ) argv;

    if ( storage )
    {
        android_extract_assets( storage );
        chdir( storage );
        file_addp( storage );
    }

    if ( standalone )
        *standalone = 1;

    if ( argc >= 2 )
        return NULL;

    if ( access( "main.dcb", R_OK ) == 0 ) return "main.dcb";
    if ( access( "hello.dcb", R_OK ) == 0 ) return "hello.dcb";

    fprintf( stderr, "Android: no main.dcb or hello.dcb in %s\n",
             storage ? storage : "(no storage)" );
    return NULL;
}
