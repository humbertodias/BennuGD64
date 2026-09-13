/* Minimal mmreg.h stub for SDL3 Windows core on nxdk. */
#ifndef __BENNUGD_XBOX_MMREG_H
#define __BENNUGD_XBOX_MMREG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef WAVE_FORMAT_PCM
#define WAVE_FORMAT_PCM 0x0001
#endif

typedef struct tWAVEFORMATEX {
  uint16_t wFormatTag;
  uint16_t nChannels;
  uint32_t nSamplesPerSec;
  uint32_t nAvgBytesPerSec;
  uint16_t nBlockAlign;
  uint16_t wBitsPerSample;
  uint16_t cbSize;
} WAVEFORMATEX;

#ifdef __cplusplus
}
#endif

#endif
