/*
 * AESND on Wii is 32 kHz. Compiled only into the wii-powerpc build.
 */

#include "mod_sound_wii.h"

void modsound_wii_adjust_rate( int * audio_rate )
{
    if ( audio_rate )
        *audio_rate = 32000;
}
