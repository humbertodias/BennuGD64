# Apple tvOS: static modules, interpreter-only app bundle (bgdi.app).
# Compile .prg with the tvos-host preset (native Linux/macOS bgdc).
# Docker: osxcross + AppleTVOS.sdk (cmake/toolchains/osxcross-tvos.cmake). Native Mac
# can still pass -G Xcode without a toolchain file.

set (PLATFORM_TVOS ON)
set (TVOS ON)

set (STATIC_MODULES ON CACHE BOOL "tvOS ships one app bundle" FORCE)
set (INTERPRETER_ONLY ON CACHE BOOL "The .app ships the interpreter; compile .prg on a host" FORCE)

# FetchContent zlib/libpng/SDL tools must not become .app bundles.
set (CMAKE_MACOSX_BUNDLE FALSE)

enable_language (OBJC)

set (SDL_SHARED OFF CACHE BOOL "" FORCE)
set (SDL_STATIC ON CACHE BOOL "" FORCE)
set (SDL_TEST OFF CACHE BOOL "" FORCE)
set (SDL_TESTS OFF CACHE BOOL "" FORCE)
set (SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
set (SDL_OPENGL OFF CACHE BOOL "" FORCE)
set (SDL_OPENGLES OFF CACHE BOOL "" FORCE)
set (SDL_HIDAPI OFF CACHE BOOL "" FORCE)
set (SDL_VIRTUAL_JOYSTICK OFF CACHE BOOL "" FORCE)

if (NOT DEFINED BENNUGD_BUNDLE_IDENTIFIER)
  set (BENNUGD_BUNDLE_IDENTIFIER "org.bennugd64.player" CACHE STRING "tvOS CFBundleIdentifier")
endif ()
if (NOT DEFINED BENNUGD_BUNDLE_NAME)
  set (BENNUGD_BUNDLE_NAME "BennuGD64" CACHE STRING "tvOS display name")
endif ()
if (NOT DEFINED BENNUGD_DEVELOPMENT_TEAM)
  set (BENNUGD_DEVELOPMENT_TEAM "" CACHE STRING "Apple Development Team ID (device builds)")
endif ()

# Unsigned Docker / Simulator runs must not wait on a signing identity.
if (CMAKE_OSX_SYSROOT MATCHES "[Ss]imulator" OR CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
  set (CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO")
endif ()

# libpng genout.cmake preprocesses with -isysroot ${CMAKE_OSX_SYSROOT}.
# The short names appletvsimulator / appletvos are not directories.
if (CMAKE_OSX_SYSROOT AND NOT IS_DIRECTORY "${CMAKE_OSX_SYSROOT}")
  execute_process (
    COMMAND xcrun --sdk "${CMAKE_OSX_SYSROOT}" --show-sdk-path
    OUTPUT_VARIABLE _tvos_sdk
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE _tvos_sdk_st
  )
  if (NOT _tvos_sdk_st EQUAL 0 OR _tvos_sdk STREQUAL "")
    message (FATAL_ERROR "xcrun could not resolve SDK '${CMAKE_OSX_SYSROOT}'")
  endif ()
  set (CMAKE_OSX_SYSROOT "${_tvos_sdk}" CACHE STRING "tvOS SDK path" FORCE)
  message (STATUS "tvOS SDK: ${CMAKE_OSX_SYSROOT}")
endif ()

if (NOT USE_LIBDES)
  message (WARNING "tvOS builds should use -DUSE_LIBDES=ON (OpenSSL is not fetched for Apple TV)")
endif ()
