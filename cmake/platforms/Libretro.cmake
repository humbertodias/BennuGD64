# Libretro core: static modules. Desktop/Android/Apple use a shared object;
# consoles that cannot dlopen produce a static archive for RetroArch.

include (FetchContent)
FetchContent_Declare (
  libretro-common
  GIT_REPOSITORY https://github.com/libretro/libretro-common.git
  GIT_TAG        0abedaac6a795c093f2e1a22f3028fca9efdf3c9
)
FetchContent_MakeAvailable ( libretro-common )
set (LIBRETRO_COMMON_DIR "${libretro-common_SOURCE_DIR}" CACHE INTERNAL "libretro-common source")
include_directories ( ${LIBRETRO_COMMON_DIR}/include )

set (STATIC_MODULES ON CACHE BOOL "Libretro core links modules into the shared object" FORCE)

set (_bennugd_libretro_static_core OFF)
if (NINTENDO_SWITCH
    OR PLATFORM_DREAMCAST OR DREAMCAST
    OR PLATFORM_PSP OR PSP
    OR PLATFORM_VITA OR VITA
    OR PLATFORM_PS2 OR PS2
    OR PLATFORM_PS3 OR PS3
    OR PLATFORM_PS4 OR PS4
    OR PLATFORM_XBOX OR XBOX
    OR PLATFORM_XBOX360 OR XBOX360
    OR NINTENDO_WII OR PLATFORM_WII
    OR EMSCRIPTEN
    OR BENNUGD_WASI)
  set (_bennugd_libretro_static_core ON)
endif ()
set (BENNUGD_LIBRETRO_STATIC_CORE ${_bennugd_libretro_static_core} CACHE BOOL
  "Build bennugd_libretro as a static archive" FORCE)

if (NOT BENNUGD_LIBRETRO_STATIC_CORE)
  set (CMAKE_POSITION_INDEPENDENT_CODE ON)
  if (NOT MSVC)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  endif ()
endif ()

add_compile_definitions (TARGET_LIBRETRO=1 LIBRETRO_CORE=1)
add_definitions (-DTARGET_LIBRETRO -DLIBRETRO_CORE=1)

# Dummy SDL only on hosts that have that backend. Console SDL ports keep
# their platform video/audio; libretro still presents frames via TARGET_LIBRETRO.
if (NOT BENNUGD_LIBRETRO_STATIC_CORE
    AND NOT ANDROID
    AND NOT CMAKE_SYSTEM_NAME STREQUAL "iOS"
    AND NOT CMAKE_SYSTEM_NAME STREQUAL "tvOS")
  set (SDL_UNIX_CONSOLE_BUILD ON CACHE BOOL "" FORCE)
  set (SDL_X11 OFF CACHE BOOL "" FORCE)
  set (SDL_WAYLAND OFF CACHE BOOL "" FORCE)
  set (SDL_KMSDRM OFF CACHE BOOL "" FORCE)
  set (SDL_COCOA OFF CACHE BOOL "" FORCE)
  set (SDL_METAL OFF CACHE BOOL "" FORCE)
  set (SDL_RENDER_METAL OFF CACHE BOOL "" FORCE)
  set (SDL_OPENGL OFF CACHE BOOL "" FORCE)
  set (SDL_OPENGLES OFF CACHE BOOL "" FORCE)
  set (SDL_VULKAN OFF CACHE BOOL "" FORCE)
  set (SDL_RENDER_GPU OFF CACHE BOOL "" FORCE)
  set (SDL_GPU OFF CACHE BOOL "" FORCE)
  set (SDL_CAMERA OFF CACHE BOOL "" FORCE)
  set (SDL_DIALOG OFF CACHE BOOL "" FORCE)
  set (SDL_TRAY OFF CACHE BOOL "" FORCE)
  set (SDL_HIDAPI OFF CACHE BOOL "" FORCE)
  set (SDL_VIRTUAL_JOYSTICK OFF CACHE BOOL "" FORCE)
  set (SDL_DUMMYVIDEO ON CACHE BOOL "" FORCE)
  set (SDL_DUMMYAUDIO ON CACHE BOOL "" FORCE)
  set (SDL_DISKAUDIO OFF CACHE BOOL "" FORCE)
  set (SDL_PIPEWIRE OFF CACHE BOOL "" FORCE)
  set (SDL_PULSEAUDIO OFF CACHE BOOL "" FORCE)
  set (SDL_ALSA OFF CACHE BOOL "" FORCE)
  set (SDL_JACK OFF CACHE BOOL "" FORCE)
  set (SDL_SNDIO OFF CACHE BOOL "" FORCE)
  set (SDL_WASAPI OFF CACHE BOOL "" FORCE)
endif ()

if (BENNUGD_LIBRETRO_STATIC_CORE)
  message (STATUS "BennuGD64: building static libretro core (bennugd_libretro)")
else ()
  message (STATUS "BennuGD64: building libretro core (bennugd_libretro)")
endif ()
