# Zeebo homebrew: static modules, interpreter only (bgdi.elf).
# Compile .prg on a host with the zeebo-host preset.
# CPU is ARM1136, soft-float, big-endian. BREW has no pthread.

set (STATIC_MODULES ON CACHE BOOL "Zeebo homebrew is a single bgdi.elf" FORCE)
set (INTERPRETER_ONLY ON CACHE BOOL "Compile .prg files with a host bgdc" FORCE)
set (USE_LIBDES ON CACHE BOOL "Use bundled DES with newlib" FORCE)
set (CMAKE_POSITION_INDEPENDENT_CODE OFF CACHE BOOL "Zeebo ELF is not a shared object" FORCE)

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
set (SDL_THREADS OFF CACHE BOOL "" FORCE)
set (SDL_PTHREADS OFF CACHE BOOL "" FORCE)
set (SDL_UNIX_CONSOLE_BUILD ON CACHE BOOL "" FORCE)
set (HAVE_SDL_TIMERS TRUE CACHE BOOL "" FORCE)
set (HAVE_SIGACTION OFF CACHE BOOL "" FORCE)
set (HAVE_SIGTIMEDWAIT OFF CACHE BOOL "" FORCE)
set (HAVE_SA_SIGACTION OFF CACHE BOOL "" FORCE)
set (HAVE_SIGNAL_H OFF CACHE BOOL "" FORCE)
set (HAVE_SIGNAL_SUPPORT OFF CACHE BOOL "" FORCE)
# newlib declares posix_spawn/vfork, and try_compile is a static library,
# so SDL would treat those as available and build the POSIX process backend.
set (HAVE_POSIX_SPAWN OFF CACHE BOOL "Zeebo newlib has no posix_spawn" FORCE)
set (LIBC_HAS_VFORK OFF CACHE BOOL "Zeebo newlib has no vfork" FORCE)

set (CMAKE_EXECUTABLE_SUFFIX ".elf")
set (CMAKE_EXECUTABLE_SUFFIX_C ".elf")

include_directories (${CMAKE_SOURCE_DIR}/platforms/zeebo/include)
