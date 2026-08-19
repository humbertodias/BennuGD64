#ifndef __MOD_WPAD_EXPORTS
#define __MOD_WPAD_EXPORTS

#include "bgddl.h"

#define WPAD_BATT        0
#define WPAD_X           1
#define WPAD_Y           2
#define WPAD_Z           3
#define WPAD_ANGLE       4
#define WPAD_PITCH       5
#define WPAD_ROLL        6
#define WPAD_ACCELX      7
#define WPAD_ACCELY      8
#define WPAD_ACCELZ      9
#define WPAD_IS_BB       10
#define WPAD_WTL         3
#define WPAD_WTR         4
#define WPAD_WBL         5
#define WPAD_WBR         6

#if defined(__BGDC__) || !defined(__STATIC__)
DLCONSTANT __bgdexport( mod_wpad, constants_def )[] =
{
    { "WPAD_BATT",      TYPE_INT,   WPAD_BATT   },
    { "WPAD_X",         TYPE_INT,   WPAD_X      },
    { "WPAD_Y",         TYPE_INT,   WPAD_Y      },
    { "WPAD_Z",         TYPE_INT,   WPAD_Z      },
    { "WPAD_ANGLE",     TYPE_INT,   WPAD_ANGLE  },
    { "WPAD_PITCH",     TYPE_INT,   WPAD_PITCH  },
    { "WPAD_ROLL",      TYPE_INT,   WPAD_ROLL   },
    { "WPAD_ACCELX",    TYPE_INT,   WPAD_ACCELX },
    { "WPAD_ACCELY",    TYPE_INT,   WPAD_ACCELY },
    { "WPAD_ACCELZ",    TYPE_INT,   WPAD_ACCELZ },
    { "WPAD_IS_BB",     TYPE_INT,   WPAD_IS_BB  },
    { "WPAD_WTL",       TYPE_INT,   WPAD_WTL    },
    { "WPAD_WTR",       TYPE_INT,   WPAD_WTR    },
    { "WPAD_WBL",       TYPE_INT,   WPAD_WBL    },
    { "WPAD_WBR",       TYPE_INT,   WPAD_WBR    },
    { NULL,             0,          0           }
};
#endif

DLSYSFUNCS  __bgdexport( mod_wpad, functions_exports )[] =
{
    FUNC( "WPAD_IS_READY", "I",  TYPE_INT,       modwpad_is_ready ),
    FUNC( "WPAD_INFO",     "II", TYPE_INT,       modwpad_info     ),
    FUNC( "WPAD_INFO_BB",  "II", TYPE_INT,       modwpad_info_bb  ),
    FUNC( "WPAD_RUMBLE",   "II", TYPE_UNDEFINED, modwpad_rumble   ),
    FUNC( 0,               0,    0,              0                )
};

char * __bgdexport( mod_wpad, modules_dependency )[] =
{
    "libjoy",
    NULL
};

#endif
