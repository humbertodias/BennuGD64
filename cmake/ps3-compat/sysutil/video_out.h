/* CELL-style names used by onesixromcom SDL3 (ps3 branch).
 * Stock PSL1GHT only ships sysutil/video.h (videoConfigure, videoState, …).
 * Keep this tree off portlibs so FetchContent libpng does not see zlib 1.2.x.
 */
#ifndef BENNUGD_PSL1GHT_VIDEO_OUT_H
#define BENNUGD_PSL1GHT_VIDEO_OUT_H

#include <sysutil/video.h>

typedef videoConfiguration videoOutConfiguration;
typedef videoState videoOutState;
typedef videoResolution videoOutResolution;
typedef videoDeviceInfo videoOutDeviceInfo;
typedef videoCallback videoOutCallback;

#define videoOutGetState videoGetState
#define videoOutGetResolution videoGetResolution
#define videoOutConfigure videoConfigure
#define videoOutGetConfiguration videoGetConfiguration
#define videoOutGetDeviceInfo videoGetDeviceInfo
#define videoOutRegisterCallback videoRegisterCallback
#define videoOutUnregisterCallback videoUnregisterCallback

#define VIDEO_OUT_BUFFER_FORMAT_XRGB VIDEO_BUFFER_FORMAT_XRGB
#define VIDEO_OUT_BUFFER_FORMAT_XBGR VIDEO_BUFFER_FORMAT_XBGR
#define VIDEO_OUT_BUFFER_FORMAT_FLOAT VIDEO_BUFFER_FORMAT_FLOAT

#define VIDEO_OUT_RESOLUTION_1080 VIDEO_RESOLUTION_1080
#define VIDEO_OUT_RESOLUTION_720 VIDEO_RESOLUTION_720
#define VIDEO_OUT_RESOLUTION_480 VIDEO_RESOLUTION_480
#define VIDEO_OUT_RESOLUTION_576 VIDEO_RESOLUTION_576

#define VIDEO_OUT_ASPECT_AUTO VIDEO_ASPECT_AUTO
#define VIDEO_OUT_ASPECT_4_3 VIDEO_ASPECT_4_3
#define VIDEO_OUT_ASPECT_16_9 VIDEO_ASPECT_16_9

#define VIDEO_OUT_PRIMARY VIDEO_PRIMARY
#define VIDEO_OUT_SECONDARY VIDEO_SECONDARY

#endif
