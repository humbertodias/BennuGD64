# Shared osxcross settings. Included from osxcross-x86_64.cmake / osxcross-arm64.cmake.
# CMakeFindBinUtils requires install_name_tool on Darwin (libpng enables ASM).

set (CMAKE_SYSTEM_NAME Darwin)
set (CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "")

if (DEFINED ENV{SDKROOT} AND EXISTS "$ENV{SDKROOT}")
  set (CMAKE_OSX_SYSROOT "$ENV{SDKROOT}" CACHE PATH "")
elseif (DEFINED ENV{OSXCROSS_SDK} AND EXISTS "$ENV{OSXCROSS_SDK}")
  set (CMAKE_OSX_SYSROOT "$ENV{OSXCROSS_SDK}" CACHE PATH "")
endif ()

set (_osxcross_bin "$ENV{OSXCROSS_TARGET_DIR}/bin")
if (NOT _osxcross_bin OR NOT EXISTS "${_osxcross_bin}")
  set (_osxcross_bin "/opt/osxcross/target/bin")
endif ()

set (_osxcross_prefixes)
if (DEFINED ENV{OSXCROSS_HOST} AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64")
  list (APPEND _osxcross_prefixes "$ENV{OSXCROSS_HOST}")
endif ()
if (CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
  list (APPEND _osxcross_prefixes aarch64-apple-darwin arm64-apple-darwin)
else ()
  list (APPEND _osxcross_prefixes x86_64-apple-darwin)
endif ()

macro (_osxcross_tool VAR tool)
  if (NOT ${VAR})
    set (_found "")
    foreach (_p ${_osxcross_prefixes})
      file (GLOB _cands
        "${_osxcross_bin}/${_p}-${tool}"
        "${_osxcross_bin}/${_p}*-${tool}"
      )
      if (_cands)
        list (SORT _cands)
        list (GET _cands 0 _found)
        break ()
      endif ()
    endforeach ()
    if (_found)
      set (${VAR} "${_found}" CACHE FILEPATH "${tool}")
    endif ()
  endif ()
endmacro ()

_osxcross_tool (CMAKE_AR ar)
_osxcross_tool (CMAKE_RANLIB ranlib)
_osxcross_tool (CMAKE_STRIP strip)
_osxcross_tool (CMAKE_NM nm)
_osxcross_tool (CMAKE_LINKER ld)
_osxcross_tool (CMAKE_INSTALL_NAME_TOOL install_name_tool)
_osxcross_tool (CMAKE_LIPO lipo)
_osxcross_tool (CMAKE_OTOOL otool)

if (NOT CMAKE_INSTALL_NAME_TOOL)
  message (FATAL_ERROR
    "osxcross install_name_tool not found in ${_osxcross_bin} "
    "(prefixes: ${_osxcross_prefixes})")
endif ()

set (CMAKE_FIND_ROOT_PATH
  ${CMAKE_FIND_ROOT_PATH}
  ${CMAKE_OSX_SYSROOT}
)

set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Mach-O try_compile executables cannot run on the Linux build host.
set (CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set (BENNUGD_OSXCROSS ON CACHE BOOL "Cross-compiling Darwin with osxcross")

# ld64 embeds an ad-hoc signature; osxcross install_name_tool does not
# regenerate it, so dyld rejects @rpath dylibs:
#   code signature not valid for use in process / library load disallowed
set (CMAKE_EXE_LINKER_FLAGS
  "${CMAKE_EXE_LINKER_FLAGS} -Wl,-headerpad_max_install_names -Wl,-no_adhoc_codesign")
set (CMAKE_SHARED_LINKER_FLAGS
  "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-headerpad_max_install_names -Wl,-no_adhoc_codesign")
set (CMAKE_MODULE_LINKER_FLAGS
  "${CMAKE_MODULE_LINKER_FLAGS} -Wl,-headerpad_max_install_names -Wl,-no_adhoc_codesign")

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

