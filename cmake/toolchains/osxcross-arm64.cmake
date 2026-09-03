# Cross-compile to macOS arm64 with osxcross (oa64-clang).
# Used by docker/Dockerfile.macos.

set (CMAKE_SYSTEM_PROCESSOR arm64)
set (CMAKE_OSX_ARCHITECTURES arm64)
set (CMAKE_C_COMPILER oa64-clang)
set (CMAKE_CXX_COMPILER oa64-clang++)
set (CMAKE_OBJC_COMPILER oa64-clang)
set (CMAKE_OBJCXX_COMPILER oa64-clang++)
set (CMAKE_ASM_COMPILER oa64-clang)

include (${CMAKE_CURRENT_LIST_DIR}/osxcross-common.cmake)
