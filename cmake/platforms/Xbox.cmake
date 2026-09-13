# Original Xbox homebrew: static interpreter XBE for nxdk.

set (STATIC_MODULES ON CACHE BOOL "Original Xbox homebrew is a single XBE" FORCE)
set (INTERPRETER_ONLY ON CACHE BOOL "Compile .prg files with a host bgdc" FORCE)
set (USE_LIBDES ON CACHE BOOL "Use bundled DES with nxdk/pdclib" FORCE)
set (CMAKE_POSITION_INDEPENDENT_CODE OFF CACHE BOOL "nxdk uses fixed-address XBE" FORCE)

set (SDL_SHARED OFF CACHE BOOL "" FORCE)
set (SDL_STATIC ON CACHE BOOL "" FORCE)
set (SDL_DYNAPI OFF CACHE BOOL "" FORCE)
set (SDL_SYSTEM_ICONV OFF CACHE BOOL "" FORCE)
set (SDL_OPENGL OFF CACHE BOOL "" FORCE)
set (SDL_OPENGLES OFF CACHE BOOL "" FORCE)
set (SDL_VULKAN OFF CACHE BOOL "" FORCE)
set (SDL_RENDER_GPU OFF CACHE BOOL "" FORCE)
set (SDL_GPU OFF CACHE BOOL "" FORCE)
set (SDL_HIDAPI OFF CACHE BOOL "" FORCE)
set (SDL_VIRTUAL_JOYSTICK OFF CACHE BOOL "" FORCE)
set (SDL_CAMERA OFF CACHE BOOL "" FORCE)
set (SDL_HAPTIC OFF CACHE BOOL "" FORCE)
set (SDL_SENSOR OFF CACHE BOOL "" FORCE)
set (SDL_VIDEO OFF CACHE BOOL "" FORCE)
set (SDL_AUDIO OFF CACHE BOOL "" FORCE)
set (SDL_JOYSTICK OFF CACHE BOOL "" FORCE)
set (SDL_UNIX_CONSOLE_BUILD ON CACHE BOOL "" FORCE)
set (HAVE_SDL_TIMERS TRUE CACHE BOOL "" FORCE)
set (HAVE_SIGACTION OFF CACHE BOOL "" FORCE)
set (HAVE_SIGTIMEDWAIT OFF CACHE BOOL "" FORCE)
set (HAVE_SA_SIGACTION OFF CACHE BOOL "" FORCE)
set (HAVE_SIGNAL_H OFF CACHE BOOL "" FORCE)
set (HAVE_SIGNAL_SUPPORT OFF CACHE BOOL "" FORCE)

set (CMAKE_EXECUTABLE_SUFFIX ".exe")
set (CMAKE_EXECUTABLE_SUFFIX_C ".exe")

# nxdk-link defaults to -stack:65536; Bennu+SDL needs far more (interpreter,
# module init). Pass after the wrapper defaults so lld uses this value.
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stack:1048576")
set (CMAKE_EXE_LINKER_FLAGS_C "${CMAKE_EXE_LINKER_FLAGS_C} -stack:1048576")
set (CMAKE_EXE_LINKER_FLAGS_CXX "${CMAKE_EXE_LINKER_FLAGS_CXX} -stack:1048576")

include_directories (BEFORE SYSTEM "${CMAKE_SOURCE_DIR}/platforms/xbox/include")

if (NOT DEFINED NXDK_DIR OR NXDK_DIR STREQUAL "")
  if (DEFINED ENV{NXDK_DIR})
    set (NXDK_DIR "$ENV{NXDK_DIR}")
  else ()
    set (NXDK_DIR "/usr/src/nxdk")
  endif ()
endif ()

# Prefer nxdk's prebuilt zlib/png; imported targets are created in FetchDeps.
set (XBOX_NXDK_LIB_DIR "${NXDK_DIR}/lib")

add_library (Xbox::Nxdk INTERFACE IMPORTED GLOBAL)
target_link_libraries (Xbox::Nxdk INTERFACE
  "${XBOX_NXDK_LIB_DIR}/libnxdk_automount_d.lib"
  "${XBOX_NXDK_LIB_DIR}/nxdk_usb.lib"
  "${XBOX_NXDK_LIB_DIR}/libnxdk_hal.lib"
  "${XBOX_NXDK_LIB_DIR}/libnxdk.lib"
  "${XBOX_NXDK_LIB_DIR}/libxboxrt.lib"
  "${XBOX_NXDK_LIB_DIR}/libwinapi.lib"
  "${XBOX_NXDK_LIB_DIR}/xboxkrnl/libxboxkrnl.lib"
  "${XBOX_NXDK_LIB_DIR}/libpdclib.lib"
)
target_include_directories (Xbox::Nxdk INTERFACE
  "${NXDK_DIR}/lib"
  "${NXDK_DIR}/lib/hal"
  "${NXDK_DIR}/lib/usb"
  "${NXDK_DIR}/lib/usb/libusbohci"
  "${NXDK_DIR}/lib/usb/libusbohci/inc"
  "${NXDK_DIR}/lib/usb/libusbohci_xbox"
  "${NXDK_DIR}/lib/winapi"
)
