# Cross-compile to Windows x86_64 with MinGW-w64.
# Used by docker/Dockerfile.windows so Windows artifacts do not need MSYS2.

set (CMAKE_SYSTEM_NAME Windows)
set (CMAKE_SYSTEM_PROCESSOR x86_64)

set (CMAKE_C_COMPILER x86_64-w64-mingw32-gcc-posix)
set (CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++-posix)
set (CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)
set (CMAKE_AR x86_64-w64-mingw32-ar)
set (CMAKE_RANLIB x86_64-w64-mingw32-ranlib)
set (CMAKE_STRIP x86_64-w64-mingw32-strip)
set (CMAKE_NM x86_64-w64-mingw32-nm)

set (CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)
set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

find_program (_mingw_pkg_config x86_64-w64-mingw32-pkg-config)
if (_mingw_pkg_config)
  set (PKG_CONFIG_EXECUTABLE "${_mingw_pkg_config}" CACHE FILEPATH "" FORCE)
endif ()
