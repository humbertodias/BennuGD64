# Cross-compile Original Xbox homebrew with the open nxdk toolchain.
#   cmake --preset xbox-i386
# Requires NXDK_DIR (normally /usr/src/nxdk inside xboxdev/nxdk).

if (DEFINED ENV{NXDK_DIR} AND IS_DIRECTORY "$ENV{NXDK_DIR}")
  set (NXDK_DIR "$ENV{NXDK_DIR}")
else ()
  set (NXDK_DIR "/usr/src/nxdk")
  set (ENV{NXDK_DIR} "${NXDK_DIR}")
endif ()

if (NOT EXISTS "${NXDK_DIR}/share/toolchain-nxdk.cmake")
  message (FATAL_ERROR "nxdk toolchain not found at ${NXDK_DIR}/share/toolchain-nxdk.cmake")
endif ()

include ("${NXDK_DIR}/share/toolchain-nxdk.cmake")

# Official toolchain marks WIN32 for WinAPI headers. Keep that for includes,
# but tell Bennu this is the Original Xbox homebrew target.
set (PLATFORM_XBOX TRUE CACHE BOOL "Build Original Xbox nxdk homebrew" FORCE)
set (XBOX TRUE CACHE BOOL "Build Original Xbox nxdk homebrew" FORCE)
set (NXDK TRUE CACHE BOOL "Building with nxdk" FORCE)

# SDL3's Windows backend needs a resource compiler; llvm-rc ships with the image.
if (EXISTS "/usr/bin/llvm-rc")
  set (CMAKE_RC_COMPILER "/usr/bin/llvm-rc" CACHE FILEPATH "" FORCE)
elseif (EXISTS "/usr/bin/llvm20-rc")
  set (CMAKE_RC_COMPILER "/usr/bin/llvm20-rc" CACHE FILEPATH "" FORCE)
endif ()
if (CMAKE_RC_COMPILER)
  set (CMAKE_RC_COMPILE_OBJECT
    "<CMAKE_RC_COMPILER> <DEFINES> <INCLUDES> <FLAGS> /fo <OBJECT> <SOURCE>"
    CACHE STRING "nxdk llvm-rc compile rule" FORCE)
endif ()

add_compile_definitions (
  TARGET_XBOX=1
  __XBOX__=1
  NXDK=1
  _WIN32=1
  WIN32=1
  SDL_DISABLE_ANALYZE_MACROS=1
  "USBH_USE_EXTERNAL_CONFIG=\"usbh_config_xbox.h\""
  "M_PI=3.14159265358979323846"
)

# Prefer PE/COFF archives from nxdk; keep try_compile from probing the host.
set (CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set_property (GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)
