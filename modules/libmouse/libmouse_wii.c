/*
 * Wiimote IR as mouse. Compiled only into the wii-powerpc build.
 */

#include "bgddl.h"
#include "dlvaracc.h"
#include "libvideo.h"
#include "libmouse_wii.h"

#include <wiiuse/wpad.h>

extern DLVARFIXUP libmouse_globals_fixup[];

enum {
    MOUSEX = 0,
    MOUSEY,
    MOUSEZ,
    MOUSEFILE,
    MOUSEGRAPH,
    MOUSEANGLE,
    MOUSESIZE,
    MOUSEFLAGS,
    MOUSEREGION,
    MOUSELEFT,
    MOUSEMIDDLE,
    MOUSERIGHT
};

void libmouse_wii_after_events( void )
{
    WPADData * wd;

    WPAD_ScanPads();
    wd = WPAD_Data( 0 );
    if ( !wd || !wd->ir.valid )
        return;

    {
        int mx = ( int ) wd->ir.x;
        int my = ( int ) wd->ir.y;

        if ( scr_width > 0 && scr_height > 0 &&
             ( scr_width != 640 || scr_height != 480 ) )
        {
            mx = mx * scr_width / 640;
            my = my * scr_height / 480;
        }
        GLOINT32( libmouse, MOUSEX ) = mx;
        GLOINT32( libmouse, MOUSEY ) = my;
        GLODWORD( libmouse, MOUSELEFT )  = ( wd->btns_h & WPAD_BUTTON_A ) ? 1 : 0;
        GLODWORD( libmouse, MOUSERIGHT ) = ( wd->btns_h & WPAD_BUTTON_B ) ? 1 : 0;
    }
}
