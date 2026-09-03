# Cross-compile to macOS x86_64 with osxcross (o64-clang).
# Used by docker/Dockerfile.macos.

set (CMAKE_SYSTEM_PROCESSOR x86_64)
set (CMAKE_OSX_ARCHITECTURES x86_64)
set (CMAKE_C_COMPILER o64-clang)
set (CMAKE_CXX_COMPILER o64-clang++)
set (CMAKE_OBJC_COMPILER o64-clang)
set (CMAKE_OBJCXX_COMPILER o64-clang++)
set (CMAKE_ASM_COMPILER o64-clang)

include (${CMAKE_CURRENT_LIST_DIR}/osxcross-common.cmake)
