# Cross-compile PlayStation 5 payloads with ps5-payload-sdk.
#   cmake --preset ps5-x86_64
# Requires PS5_PAYLOAD_SDK (prospero-clang, target sysroot, sce stubs).

if (DEFINED ENV{PS5_PAYLOAD_SDK} AND IS_DIRECTORY "$ENV{PS5_PAYLOAD_SDK}")
  set (PS5_PAYLOAD_SDK "$ENV{PS5_PAYLOAD_SDK}")
else ()
  set (PS5_PAYLOAD_SDK "/opt/ps5-payload-sdk")
  set (ENV{PS5_PAYLOAD_SDK} "${PS5_PAYLOAD_SDK}")
endif ()

if (NOT EXISTS "${PS5_PAYLOAD_SDK}/toolchain/prospero.cmake")
  message (FATAL_ERROR
    "PS5 payload SDK not found under ${PS5_PAYLOAD_SDK} "
    "(set PS5_PAYLOAD_SDK)")
endif ()

include ("${PS5_PAYLOAD_SDK}/toolchain/prospero.cmake")

# The SDK toolchain turns makefile verbosity on. Match the other presets.
set (CMAKE_VERBOSE_MAKEFILE OFF CACHE BOOL "" FORCE)

# Prospero CRT cannot run a host try_compile executable.
set (CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# The sysroot has libpthread; the probe cannot execute.
set (THREADS_PTHREAD_ARG "0" CACHE STRING "" FORCE)
set (CMAKE_THREAD_LIBS_INIT "-lpthread" CACHE STRING "" FORCE)
set (CMAKE_HAVE_THREADS_LIBRARY 1 CACHE INTERNAL "" FORCE)
set (CMAKE_USE_PTHREADS_INIT 1 CACHE INTERNAL "" FORCE)
set (THREADS_HAVE_PTHREAD_ARG FALSE CACHE BOOL "" FORCE)
set (CMAKE_SKIP_RPATH TRUE CACHE BOOL "" FORCE)
set (CMAKE_SKIP_INSTALL_RPATH TRUE CACHE BOOL "" FORCE)

set (PLATFORM_PS5 TRUE CACHE BOOL "Build PlayStation 5 homebrew" FORCE)
set (PS5 TRUE CACHE BOOL "Build PlayStation 5 homebrew" FORCE)
option (BENNUGD_PS5_NATIVE_TITLE "Build a native PS5 application for the PKG" OFF)

string (APPEND CMAKE_C_FLAGS_INIT
  " -D_GNU_SOURCE=1 -D__PS5__=1 -D__PROSPERO__=1 -DPS5=1")
string (APPEND CMAKE_CXX_FLAGS_INIT
  " -D_GNU_SOURCE=1 -D__PS5__=1 -D__PROSPERO__=1 -DPS5=1 -fno-rtti -fno-exceptions")
string (APPEND CMAKE_C_FLAGS_RELEASE " -O2")
string (APPEND CMAKE_CXX_FLAGS_RELEASE " -O2")
