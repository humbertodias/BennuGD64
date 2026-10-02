/*
 * Zeebo interpreter entry. The console is BREW: AEEMod_Load only publishes
 * an IModule and returns 0. The emulator then calls CreateInstance and
 * delivers EVT_APP_START, and that is where main() runs.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "main_zeebo.h"

extern int main( int argc, char * argv[] );

/* Class exported by platforms/zeebo/bgdi.brx. */
#define ZEEBO_APPLET_CLSID 0x0100B6D1u
#define ZEEBO_SUCCESS 0
#define ZEEBO_ECLASSNOTSUPPORT 20
#define EVT_APP_START 0

typedef struct
{
    void * vtbl;
    int refs;
    void * shell;
} zeebo_module;

typedef struct
{
    void * vtbl;
    int refs;
} zeebo_applet;

extern void zeebo_set_shell( void * shell );
extern void zeebo_set_display( void * display );

static int zeebo_file_exists( const char * path )
{
    FILE * fp = fopen( path, "rb" );
    if ( !fp ) return 0;
    fclose( fp );
    return 1;
}

char * bgdi_zeebo_startup( int argc, char * argv[], int * standalone )
{
    static const char * candidates[] = {
        "main.dcb",
        "SorR.dat",
        NULL
    };
    int i;

    if ( standalone ) *standalone = 1;

    if ( argc >= 2 && argv && argv[1] && zeebo_file_exists( argv[1] ) )
        return argv[1];

    for ( i = 0; candidates[i]; i++ )
    {
        if ( zeebo_file_exists( candidates[i] ) )
            return ( char * )candidates[i];
    }
    return NULL;
}

extern void __libc_init_array( void );

/* -nostartfiles omits crti. newlib still calls these from __libc_init_array. */
void _init( void ) {}
void _fini( void ) {}

/* libgcc unwind and newlib fini are referenced, but not linked into a
 * nostartfiles module. elf2mod rejects relocations to undefined symbols. */
void __libc_fini( void ) {}
void __cxa_type_match( void ) {}
void __cxa_begin_cleanup( void ) {}
void __cxa_call_unexpected( void ) {}
void __gnu_Unwind_Find_exidx( void ) {}

static int zeebo_mod_addref( void * obj )
{
    zeebo_module * mod = obj;
    mod->refs++;
    return mod->refs;
}

static int zeebo_mod_release( void * obj )
{
    zeebo_module * mod = obj;
    if ( mod->refs > 0 ) mod->refs--;
    return mod->refs;
}

static int zeebo_app_addref( void * obj )
{
    zeebo_applet * app = obj;
    app->refs++;
    return app->refs;
}

static int zeebo_app_release( void * obj )
{
    zeebo_applet * app = obj;
    if ( app->refs > 0 ) app->refs--;
    return app->refs;
}

static int zeebo_app_event( void * applet, uint32_t evt, uint32_t wparam, uint32_t dwparam )
{
    static char * argv[] = { "bgdi.elf", NULL };
    static int started;

    ( void )applet;
    ( void )wparam;
    if ( evt == EVT_APP_START && dwparam )
        zeebo_set_display( *( void ** )( ( uintptr_t )dwparam + 8 ) );
    if ( evt == EVT_APP_START && !started )
    {
        started = 1;
        main( 1, argv );
    }
    return 1;
}

static void * zeebo_app_vt[3];
static zeebo_applet zeebo_app;

static int zeebo_mod_create( void * mod, void * shell, uint32_t clsid, void ** out )
{
    ( void )mod;
    ( void )shell;
    if ( clsid != ZEEBO_APPLET_CLSID ) return ZEEBO_ECLASSNOTSUPPORT;
    zeebo_app_vt[0] = ( void * )zeebo_app_addref;
    zeebo_app_vt[1] = ( void * )zeebo_app_release;
    zeebo_app_vt[2] = ( void * )zeebo_app_event;
    zeebo_app.vtbl = zeebo_app_vt;
    zeebo_app.refs = 1;
    if ( out ) *out = &zeebo_app;
    return ZEEBO_SUCCESS;
}

static int zeebo_mod_free( void * mod )
{
    ( void )mod;
    return ZEEBO_SUCCESS;
}

static void * zeebo_mod_vt[4];
static zeebo_module zeebo_mod;

int AEEMod_Load( void * shell, void * helper, void ** mod )
{
    ( void )helper;
    zeebo_set_shell( shell );
    __libc_init_array();
    zeebo_mod_vt[0] = ( void * )zeebo_mod_addref;
    zeebo_mod_vt[1] = ( void * )zeebo_mod_release;
    zeebo_mod_vt[2] = ( void * )zeebo_mod_create;
    zeebo_mod_vt[3] = ( void * )zeebo_mod_free;
    zeebo_mod.vtbl = zeebo_mod_vt;
    zeebo_mod.refs = 1;
    zeebo_mod.shell = shell;
    if ( mod ) *mod = &zeebo_mod;
    return ZEEBO_SUCCESS;
}
