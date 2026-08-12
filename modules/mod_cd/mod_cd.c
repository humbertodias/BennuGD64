/*
 *  Copyright © 2006-2013 SplinterGU (Fenix/Bennugd)
 *  Copyright © 2002-2006 Fenix Team (Fenix)
 *  Copyright © 1999-2002 José Luis Cebrián Pagüe (Fenix)
 *
 *  This file is part of Bennu - Game Development
 *
 *  This software is provided 'as-is', without any express or implied
 *  warranty. In no event will the authors be held liable for any damages
 *  arising from the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *     1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 *
 *     2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 *
 *     3. This notice may not be removed or altered from any source
 *     distribution.
 *
 */

/* --------------------------------------------------------------------------- */
/* SDL2 removed the CD-ROM API; keep the module as a no-op stub.               */
/* --------------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bgddl.h"
#include "bgdrtm.h"
#include "xstrings.h"
#include "dlvaracc.h"

/* ----------------------------------------------------------------- */

enum {
    CD_TRACK = 0,
    CD_FRAME,
    CD_TRACKS,
    CD_MINUTE,
    CD_SECOND,
    CD_SUBFRAME,
    CD_MINUTES,
    CD_SECONDS,
    CD_FRAMES,
    CD_TRACKINFO
};

/* ----------------------------------------------------------------- */

DLVARFIXUP  __bgdexport( mod_cd, globals_fixup )[] =
{
    { "cdinfo.current_track", NULL, -1, -1 },
    { "cdinfo.current_frame", NULL, -1, -1 },
    { "cdinfo.tracks", NULL, -1, -1 },
    { "cdinfo.minute", NULL, -1, -1 },
    { "cdinfo.second", NULL, -1, -1 },
    { "cdinfo.subframe", NULL, -1, -1 },
    { "cdinfo.minutes", NULL, -1, -1 },
    { "cdinfo.seconds", NULL, -1, -1 },
    { "cdinfo.subframes", NULL, -1, -1 },
    { "cdinfo.track", NULL, -1, -1 },
    { NULL, NULL, -1, -1 }
};

/* ----------------------------------------------------------------- */

static int modcd_drives( INSTANCE * my, intptr_t * params )
{
    (void)my; (void)params;
    return 0;
}

/* --------------------------------------------------------------------------- */
/**
   int CD_STATUS (int CD)
   Returns the status of a CD (using SDL constants)
 **/

static int modcd_status( INSTANCE * my, intptr_t * params )
{
    (void)my; (void)params;
    return 0;
}

/* --------------------------------------------------------------------------- */
/**
   string CD_NAME (int CD)
   Returns a human-readable string with the name of a CD drive
 **/

static int modcd_name( INSTANCE * my, intptr_t * params )
{
    int result;
    (void)my; (void)params;
    result = string_new( "" );
    string_use( result );
    return result;
}

/* --------------------------------------------------------------------------- */
/**
   CD_GETINFO(int CD)
   Fills the global structure CD with information about the current CD
   Returns 1 if there is a valid CD in the drive or 0 otherwise
 **/

static int modcd_getinfo( INSTANCE * my, intptr_t * params )
{
    (void)my; (void)params;
    return 0;
}

/* --------------------------------------------------------------------------- */
/**
   CD_PLAY (int CD, int TRACK)
   Starts playing a track of the given CD
 **/

static int modcd_play( INSTANCE * my, intptr_t * params )
{
    (void)my; (void)params;
    return 0;
}

/* --------------------------------------------------------------------------- */
/**
   CD_PLAY (int CD, int TRACK, int NUMTRACKS)
   Plays a series of tracks of the CD
 **/

static int modcd_playtracks( INSTANCE * my, intptr_t * params )
{
    (void)my; (void)params;
    return 0;
}

/* --------------------------------------------------------------------------- */
/**
   CD_EJECT (int CD)
   Ejects a CD
 **/

static int modcd_eject( INSTANCE * my, intptr_t * params )
{
    (void)my; (void)params;
    return 0;
}

/* --------------------------------------------------------------------------- */
/**
   CD_PAUSE (int CD)
   Pauses the CD playing
 **/

static int modcd_pause( INSTANCE * my, intptr_t * params )
{
    (void)my; (void)params;
    return 0;
}

/* --------------------------------------------------------------------------- */
/**
   CD_RESUME (int CD)
   Resumes a CD in pause
 **/

static int modcd_resume( INSTANCE * my, intptr_t * params )
{
    (void)my; (void)params;
    return 0;
}

/* --------------------------------------------------------------------------- */
/**
   CD_STOP (int CD)
   Stops the CD
 **/

static int modcd_stop( INSTANCE * my, intptr_t * params )
{
    (void)my; (void)params;
    return 0;
}

/* --------------------------------------------------------------------------- */

void  __bgdexport( mod_cd, module_initialize )()
{
}

void  __bgdexport( mod_cd, module_finalize )()
{
}

/* --------------------------------------------------------------------------- */

#include "mod_cd_exports.h"

/* --------------------------------------------------------------------------- */
