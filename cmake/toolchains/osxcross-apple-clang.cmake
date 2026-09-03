# Host Clang targeting iOS/tvOS. Do not use oa64-clang: the osxcross wrapper
# always injects -mmacosx-version-min, which Clang rejects next to
# -mtvos-version-min / -miphoneos-version-min.

unset (ENV{MACOSX_DEPLOYMENT_TARGET})
unset (ENV{OSX_VERSION_MIN})

if (NOT CMAKE_C_COMPILER)
  find_program (_bennugd_clang NAMES clang-18 clang)
  if (NOT _bennugd_clang)
    message (FATAL_ERROR "clang not found (need host Clang to target Apple)")
  endif ()
  set (CMAKE_C_COMPILER "${_bennugd_clang}")
  set (CMAKE_OBJC_COMPILER "${_bennugd_clang}")
  set (CMAKE_ASM_COMPILER "${_bennugd_clang}")
endif ()

if (NOT CMAKE_CXX_COMPILER)
  find_program (_bennugd_clangxx NAMES clang++-18 clang++)
  if (NOT _bennugd_clangxx)
    message (FATAL_ERROR "clang++ not found (need host Clang to target Apple)")
  endif ()
  set (CMAKE_CXX_COMPILER "${_bennugd_clangxx}")
  set (CMAKE_OBJCXX_COMPILER "${_bennugd_clangxx}")
endif ()
