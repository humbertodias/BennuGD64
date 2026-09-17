# Libretro core: static modules, PIC, SDL3 dummy video/audio (no native window).

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
set (CMAKE_POSITION_INDEPENDENT_CODE ON)
if (NOT MSVC)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif ()

add_compile_definitions (TARGET_LIBRETRO=1 LIBRETRO_CORE=1)
add_definitions (-DTARGET_LIBRETRO -DLIBRETRO_CORE=1)

# SDL3: software dummy backend so the core does not open a host window.
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

message (STATUS "BennuGD64: building libretro core (bennugd_libretro)")
