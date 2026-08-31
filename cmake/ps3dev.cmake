# Cross-compile PlayStation 3 homebrew with ps3dev / PSL1GHT.
#   cmake --preset ps3-ppu
# Requires PS3DEV (typically /usr/local/ps3dev) and PSL1GHT=$PS3DEV.

if (DEFINED ENV{PS3DEV} AND IS_DIRECTORY "$ENV{PS3DEV}")
  set (PS3DEV "$ENV{PS3DEV}")
else ()
  set (PS3DEV "/usr/local/ps3dev")
  set (ENV{PS3DEV} "${PS3DEV}")
endif ()

if (NOT DEFINED ENV{PSL1GHT} OR NOT IS_DIRECTORY "$ENV{PSL1GHT}")
  set (ENV{PSL1GHT} "${PS3DEV}")
endif ()

set (CMAKE_SYSTEM_NAME Generic)
set (CMAKE_SYSTEM_VERSION 1)
set (CMAKE_SYSTEM_PROCESSOR powerpc64)
set (CMAKE_CROSSCOMPILING TRUE)

set (_bennugd_ps3_gcc "")
foreach ( _cand
    "${PS3DEV}/ppu/bin/powerpc64-ps3-elf-gcc"
    "${PS3DEV}/bin/powerpc64-ps3-elf-gcc"
    "${PS3DEV}/ppu/bin/ppu-gcc"
)
  if (EXISTS "${_cand}")
    set (_bennugd_ps3_gcc "${_cand}")
    break ()
  endif ()
endforeach ()

if (_bennugd_ps3_gcc STREQUAL "")
  message (FATAL_ERROR "ps3dev PPU gcc not found under ${PS3DEV}/ppu/bin")
endif ()

get_filename_component (_bennugd_ps3_bindir "${_bennugd_ps3_gcc}" DIRECTORY)
set (CMAKE_C_COMPILER "${_bennugd_ps3_gcc}")
if (EXISTS "${_bennugd_ps3_bindir}/powerpc64-ps3-elf-g++")
  set (CMAKE_CXX_COMPILER "${_bennugd_ps3_bindir}/powerpc64-ps3-elf-g++")
elseif (EXISTS "${_bennugd_ps3_bindir}/ppu-g++")
  set (CMAKE_CXX_COMPILER "${_bennugd_ps3_bindir}/ppu-g++")
endif ()

set (CMAKE_FIND_ROOT_PATH
  "${PS3DEV}/ppu"
  "${PS3DEV}/portlibs/ppu"
)
set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# try_compile executables need PSL1GHT CRT; test as static archives instead.
set (CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set (PLATFORM_PS3 TRUE CACHE BOOL "Build PlayStation 3 homebrew" FORCE)
set (PS3 TRUE CACHE BOOL "Build PlayStation 3 homebrew" FORCE)

# SDL3's CMakeLists uses elseif(PS3); PSL1GHT headers need __PS3__.
string (APPEND CMAKE_C_FLAGS_INIT
  " -D_GNU_SOURCE=1 -DPS3 -D__PS3__ -D__PPU__ -mcpu=cell -mhard-float -fmodulo-sched -ffunction-sections -fdata-sections")
string (APPEND CMAKE_CXX_FLAGS_INIT
  " -D_GNU_SOURCE=1 -DPS3 -D__PS3__ -D__PPU__ -mcpu=cell -mhard-float -fno-rtti -fno-exceptions")
string (APPEND CMAKE_C_FLAGS_RELEASE " -O2")
string (APPEND CMAKE_CXX_FLAGS_RELEASE " -O2")

string (APPEND CMAKE_C_FLAGS_INIT
  " -I${PS3DEV}/ppu/include -I${PS3DEV}/ppu/include/simdmath -I${PS3DEV}/portlibs/ppu/include")
string (APPEND CMAKE_CXX_FLAGS_INIT
  " -I${PS3DEV}/ppu/include -I${PS3DEV}/ppu/include/simdmath -I${PS3DEV}/portlibs/ppu/include")

string (APPEND CMAKE_EXE_LINKER_FLAGS_INIT
  " -Wl,-zmax-page-size=128 -L${PS3DEV}/ppu/lib -L${PS3DEV}/ppu/ppu/lib -L${PS3DEV}/portlibs/ppu/lib")
