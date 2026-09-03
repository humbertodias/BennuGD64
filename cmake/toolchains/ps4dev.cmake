# Cross-compile PlayStation 4 homebrew with OpenOrbis.
#   cmake --preset ps4-x86_64
# Requires OO_PS4_TOOLCHAIN (headers, stubs, link.x, create-fself, PkgTool).

if (DEFINED ENV{OO_PS4_TOOLCHAIN} AND IS_DIRECTORY "$ENV{OO_PS4_TOOLCHAIN}")
  set (OO_PS4_TOOLCHAIN "$ENV{OO_PS4_TOOLCHAIN}")
else ()
  set (OO_PS4_TOOLCHAIN "/opt/OpenOrbis/PS4Toolchain")
  set (ENV{OO_PS4_TOOLCHAIN} "${OO_PS4_TOOLCHAIN}")
endif ()

if (NOT IS_DIRECTORY "${OO_PS4_TOOLCHAIN}/include" OR NOT EXISTS "${OO_PS4_TOOLCHAIN}/link.x")
  message (FATAL_ERROR
    "OpenOrbis PS4 toolchain not found under ${OO_PS4_TOOLCHAIN} "
    "(set OO_PS4_TOOLCHAIN)")
endif ()

# OpenOrbis libc headers are musl (Linux-shaped); the clang target triple is
# still x86_64-pc-freebsd12-elf for Orbis ABI / link.x. Use Linux so SDL3
# picks <endian.h> / pthread paths that match the sysroot (not FreeBSD's
# <sys/endian.h>).
set (CMAKE_SYSTEM_NAME Linux)
set (CMAKE_SYSTEM_VERSION 1)
set (CMAKE_SYSTEM_PROCESSOR x86_64)
set (CMAKE_CROSSCOMPILING TRUE)

find_program (_bennugd_ps4_clang NAMES clang clang-18 clang-17 clang-16 REQUIRED)
find_program (_bennugd_ps4_clangxx NAMES clang++ clang++-18 clang++-17 clang++-16 REQUIRED)
find_program (_bennugd_ps4_lld NAMES ld.lld ld.lld-18 ld.lld-17 ld.lld-16 REQUIRED)

set (CMAKE_C_COMPILER "${_bennugd_ps4_clang}")
set (CMAKE_CXX_COMPILER "${_bennugd_ps4_clangxx}")
set (CMAKE_LINKER "${_bennugd_ps4_lld}")
set (CMAKE_C_COMPILER_TARGET "x86_64-pc-freebsd12-elf")
set (CMAKE_CXX_COMPILER_TARGET "x86_64-pc-freebsd12-elf")

set (CMAKE_FIND_ROOT_PATH "${OO_PS4_TOOLCHAIN}")
set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Orbis CRT cannot link a host try_compile executable.
set (CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# OpenOrbis ships libpthread.a; SDL3's CheckPTHREAD needs these when
# try_compile cannot run a pthread probe executable.
set (THREADS_PTHREAD_ARG "0" CACHE STRING "" FORCE)
set (CMAKE_THREAD_LIBS_INIT "-lpthread" CACHE STRING "" FORCE)
set (CMAKE_HAVE_THREADS_LIBRARY 1 CACHE INTERNAL "" FORCE)
set (CMAKE_USE_PTHREADS_INIT 1 CACHE INTERNAL "" FORCE)

set (PLATFORM_PS4 TRUE CACHE BOOL "Build PlayStation 4 homebrew" FORCE)
set (PS4 TRUE CACHE BOOL "Build PlayStation 4 homebrew" FORCE)

# Match OpenOrbis sample CFLAGS / LDFLAGS (clang + ld.lld + link.x + crt1.o).
# ps4-compat: FreeBSD-shaped headers missing from the musl sysroot (sys/endian.h).
# -fshort-wchar: FreeBSD clang defaults wchar_t to 4 bytes; OpenOrbis musl
# typedefs it as unsigned short (2). SDL asserts sizeof(wchar_t)==__SIZEOF_WCHAR_T__.
# -U__FreeBSD__: keep the FreeBSD clang target for Orbis ABI, but make SDL3 /
# autoconf see Linux+musl (sysroot has <endian.h>, not <sys/sysctl.h>).
string (APPEND CMAKE_C_FLAGS_INIT
  " --target=x86_64-pc-freebsd12-elf -fPIC -funwind-tables -fshort-wchar"
  " -U__FreeBSD__ -U__FreeBSD_kernel__ -D__linux__=1"
  " -D_GNU_SOURCE=1 -D_REENTRANT -D__ORBIS__=1 -D__PS4__=1 -DPS4=1"
  " -isysroot ${OO_PS4_TOOLCHAIN} -isystem ${OO_PS4_TOOLCHAIN}/include"
  " -I${CMAKE_CURRENT_LIST_DIR}/ps4-compat")
string (APPEND CMAKE_CXX_FLAGS_INIT
  " --target=x86_64-pc-freebsd12-elf -fPIC -funwind-tables -fno-rtti -fno-exceptions -fshort-wchar"
  " -U__FreeBSD__ -U__FreeBSD_kernel__ -D__linux__=1"
  " -D_GNU_SOURCE=1 -D_REENTRANT -D__ORBIS__=1 -D__PS4__=1 -DPS4=1"
  " -isysroot ${OO_PS4_TOOLCHAIN} -isystem ${OO_PS4_TOOLCHAIN}/include"
  " -isystem ${OO_PS4_TOOLCHAIN}/include/c++/v1"
  " -I${CMAKE_CURRENT_LIST_DIR}/ps4-compat")
string (APPEND CMAKE_C_FLAGS_RELEASE " -O2")
string (APPEND CMAKE_CXX_FLAGS_RELEASE " -O2")

string (APPEND CMAKE_EXE_LINKER_FLAGS_INIT
  " -m elf_x86_64 -pie --script ${OO_PS4_TOOLCHAIN}/link.x --eh-frame-hdr"
  " -L${OO_PS4_TOOLCHAIN}/lib ${OO_PS4_TOOLCHAIN}/lib/crt1.o")

# Drive ld.lld directly (OpenOrbis samples). Do not pass <FLAGS> — those are
# clang compile flags (--target, -isysroot, -D…) and ld.lld rejects them.
# Only <LINK_FLAGS>: CMAKE_C_LINK_FLAGS often duplicates EXE_LINKER_FLAGS.
set (CMAKE_C_LINK_EXECUTABLE
  "<CMAKE_LINKER> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
set (CMAKE_CXX_LINK_EXECUTABLE
  "<CMAKE_LINKER> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

# FindThreads would otherwise add -pthread (clang flag) to the link line.
set (THREADS_HAVE_PTHREAD_ARG FALSE CACHE BOOL "" FORCE)
set (CMAKE_SKIP_RPATH TRUE CACHE BOOL "" FORCE)
set (CMAKE_SKIP_INSTALL_RPATH TRUE CACHE BOOL "" FORCE)
