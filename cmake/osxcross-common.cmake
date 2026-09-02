# Shared osxcross settings. Included from osxcross-x86_64.cmake / osxcross-arm64.cmake.
# CMakeFindBinUtils requires install_name_tool on Darwin (libpng enables ASM).

set (CMAKE_SYSTEM_NAME Darwin)
set (CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "")

if (DEFINED ENV{SDKROOT} AND EXISTS "$ENV{SDKROOT}")
  set (CMAKE_OSX_SYSROOT "$ENV{SDKROOT}" CACHE PATH "")
elseif (DEFINED ENV{OSXCROSS_SDK} AND EXISTS "$ENV{OSXCROSS_SDK}")
  set (CMAKE_OSX_SYSROOT "$ENV{OSXCROSS_SDK}" CACHE PATH "")
endif ()

include (${CMAKE_CURRENT_LIST_DIR}/osxcross-tools.cmake)

# Darwin compiler-rt builtins (__isPlatformVersionAtLeast) if osxcross built them.
file (GLOB _osxcross_rt
  "${_osxcross_bin}/../lib/clang/*/lib/darwin/libclang_rt.osx.a"
  "${_osxcross_bin}/../lib/darwin/libclang_rt.osx.a"
)
if (_osxcross_rt)
  list (SORT _osxcross_rt)
  list (GET _osxcross_rt 0 _osxcross_rt)
  set (BENNUGD_OSXCROSS_COMPILER_RT "${_osxcross_rt}" CACHE FILEPATH "")
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${BENNUGD_OSXCROSS_COMPILER_RT}")
  set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${BENNUGD_OSXCROSS_COMPILER_RT}")
  set (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${BENNUGD_OSXCROSS_COMPILER_RT}")
endif ()
