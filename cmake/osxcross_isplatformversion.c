/*
 * Clang emits __isPlatformVersionAtLeast for ObjC @available / __builtin_available.
 * Native Xcode links compiler-rt; osxcross often does not, so SDL's Cocoa/camera
 * objects fail to link on arm64. Always-true is OK when MACOSX_DEPLOYMENT_TARGET
 * is already our floor (see cmake/osxcross-common.cmake).
 */
#include <stdint.h>

int32_t __isPlatformVersionAtLeast(uint32_t platform, uint32_t major, uint32_t minor,
                                   uint32_t subminor)
{
    (void) platform;
    (void) major;
    (void) minor;
    (void) subminor;
    return 1;
}

int32_t __isOSVersionAtLeast(int32_t major, int32_t minor, int32_t subminor)
{
    (void) major;
    (void) minor;
    (void) subminor;
    return 1;
}
