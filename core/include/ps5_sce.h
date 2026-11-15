/*
 * Minimal Prospero prototypes. ps5-payload-sdk ships link stubs
 * (libSce*.so) without these C headers. Video calls follow the public
 * homebrew sequence in ps5-payload-hbldr; the pad prefix matches
 * scePadReadState (buttons, then sticks).
 */

#ifndef __PS5_SCE_H
#define __PS5_SCE_H

#include <stddef.h>
#include <stdint.h>

#define SCE_PAD_PORT_TYPE_STANDARD 0

#define SCE_PAD_BUTTON_L3        0x00000002u
#define SCE_PAD_BUTTON_R3        0x00000004u
#define SCE_PAD_BUTTON_OPTIONS   0x00000008u
#define SCE_PAD_BUTTON_UP        0x00000010u
#define SCE_PAD_BUTTON_RIGHT     0x00000020u
#define SCE_PAD_BUTTON_DOWN      0x00000040u
#define SCE_PAD_BUTTON_LEFT      0x00000080u
#define SCE_PAD_BUTTON_L2        0x00000100u
#define SCE_PAD_BUTTON_R2        0x00000200u
#define SCE_PAD_BUTTON_L1        0x00000400u
#define SCE_PAD_BUTTON_R1        0x00000800u
#define SCE_PAD_BUTTON_TRIANGLE  0x00001000u
#define SCE_PAD_BUTTON_CIRCLE    0x00002000u
#define SCE_PAD_BUTTON_CROSS     0x00004000u
#define SCE_PAD_BUTTON_SQUARE    0x00008000u
#define SCE_PAD_BUTTON_TOUCH_PAD 0x00100000u

#define SCE_VIDEO_OUT_PIXEL_RGBA8 0x8000000022000000ull
#define SCE_VIDEO_OUT_BUS_MAIN    0
#define SCE_VIDEO_OUT_FLIP_VSYNC  1u

typedef struct ScePadAnalogStick
{
    uint8_t x;
    uint8_t y;
} ScePadAnalogStick;

typedef struct ScePadData
{
    uint32_t buttons;
    ScePadAnalogStick leftStick;
    ScePadAnalogStick rightStick;
    uint8_t reserved[ 240 ];
} ScePadData;

typedef struct SceVideoOutBuffer
{
    void * data;
    uint64_t reserved[ 3 ];
} SceVideoOutBuffer;

typedef struct SceVideoOutStatus
{
    uint32_t resolution;
    uint32_t reserved0;
    uint64_t reserved1[ 5 ];
} SceVideoOutStatus;

typedef struct SceVideoOutBufferAttribute2
{
    uint8_t raw[ 80 ];
} SceVideoOutBufferAttribute2;

int sceUserServiceInitialize( void * params );
int sceUserServiceGetInitialUser( int * user_id );
int sceUserServiceGetForegroundUser( int * user_id );

int scePadInit( void );
int scePadOpen( int user_id, int type, int index, const void * param );
int scePadReadState( int handle, ScePadData * data );

int sceSystemServiceHideSplashScreen( void );

uint64_t sceKernelGetProcessTime( void );
int sceKernelUsleep( unsigned int microseconds );
int sceKernelCreateEqueue( void ** queue, const char * name );
int sceKernelDeleteEqueue( void * queue );
int sceKernelWaitEqueue( void * queue, void * event, int count, int * out, void * timeout );
int sceKernelAllocateMainDirectMemory( size_t len, size_t alignment, int type, intptr_t * phys );
int sceKernelMapDirectMemory( void ** addr, size_t len, int prot, int flags, intptr_t phys, size_t alignment );
int sceKernelReleaseDirectMemory( intptr_t phys, size_t len );
int sceKernelMunmap( void * addr, size_t len );

int sceVideoOutOpen( int user_id, int bus, int index, const void * param );
int sceVideoOutClose( int handle );
int sceVideoOutGetOutputStatus( int handle, SceVideoOutStatus * status );
int sceVideoOutAddFlipEvent( void * queue, int handle, void * param );
int sceVideoOutSetFlipRate( int handle, int rate );
int sceVideoOutSubmitFlip( int handle, int buffer_index, unsigned flip_mode, int64_t flip_arg );
void sceVideoOutSetBufferAttribute2( SceVideoOutBufferAttribute2 * attribute,
                                     uint64_t pixel_format, uint32_t tiling,
                                     uint32_t width, uint32_t height,
                                     uint64_t option, uint32_t dcc_control,
                                     uint64_t dcc_clear );
int sceVideoOutRegisterBuffers2( int handle, int set_index, int start_index,
                                 SceVideoOutBuffer * buffers, int count,
                                 SceVideoOutBufferAttribute2 * attribute,
                                 int flags, void * option );
int sceVideoOutUnregisterBuffers( int handle, int start_index );

#endif
