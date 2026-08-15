# Resolve wasi-sdk and set CMAKE_TOOLCHAIN_FILE. Included before project() when
# BENNUGD_WASI=ON:
#   cmake -B build-wasi -DBENNUGD_WASI=ON
#   cmake --build build-wasi --target bgdc
#
# Honors WASI_SDK_PATH if it already points at a usable SDK.
# If CMAKE_TOOLCHAIN_FILE already selects wasi-sdk, skip the download.

if (CMAKE_TOOLCHAIN_FILE AND EXISTS "${CMAKE_TOOLCHAIN_FILE}")
  message (STATUS "Using existing WASI toolchain: ${CMAKE_TOOLCHAIN_FILE}")
  return ()
endif ()

set (BENNUGD_WASI_SDK_VERSION "33" CACHE STRING "wasi-sdk major version to download")
set (BENNUGD_WASI_SDK_VERSION_FULL "${BENNUGD_WASI_SDK_VERSION}.0" CACHE STRING "wasi-sdk full version to download")

get_filename_component (_bennu_root "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
  set (_wasi_os "macos")
elseif (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
  set (_wasi_os "windows")
else ()
  set (_wasi_os "linux")
endif ()

if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "arm64|aarch64|ARM64")
  set (_wasi_arch "arm64")
else ()
  set (_wasi_arch "x86_64")
endif ()

set (_sdk_dir "")
if (DEFINED ENV{WASI_SDK_PATH} AND EXISTS "$ENV{WASI_SDK_PATH}/bin/clang")
  set (_sdk_dir "$ENV{WASI_SDK_PATH}")
elseif (DEFINED WASI_SDK_PATH AND EXISTS "${WASI_SDK_PATH}/bin/clang")
  set (_sdk_dir "${WASI_SDK_PATH}")
endif ()

if (_sdk_dir STREQUAL "")
  set (_sdk_dir "${_bennu_root}/.deps/wasi-sdk-${BENNUGD_WASI_SDK_VERSION_FULL}-${_wasi_arch}-${_wasi_os}")
endif ()

if (NOT EXISTS "${_sdk_dir}/bin/clang")
  set (_tarball "wasi-sdk-${BENNUGD_WASI_SDK_VERSION_FULL}-${_wasi_arch}-${_wasi_os}.tar.gz")
  set (_url "https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-${BENNUGD_WASI_SDK_VERSION}/${_tarball}")
  set (_archive "${_bennu_root}/.deps/${_tarball}")
  file (MAKE_DIRECTORY "${_bennu_root}/.deps")
  message (STATUS "Downloading wasi-sdk ${BENNUGD_WASI_SDK_VERSION_FULL} from ${_url}")
  file (DOWNLOAD "${_url}" "${_archive}"
    SHOW_PROGRESS
    TIMEOUT 300
    TLS_VERIFY ON
    STATUS _dl_status
  )
  list (GET _dl_status 0 _dl_code)
  list (GET _dl_status 1 _dl_msg)
  if (NOT _dl_code EQUAL 0)
    message (FATAL_ERROR "Failed to download wasi-sdk: ${_dl_msg}")
  endif ()
  execute_process (
    COMMAND "${CMAKE_COMMAND}" -E tar xzf "${_archive}"
    WORKING_DIRECTORY "${_bennu_root}/.deps"
    RESULT_VARIABLE _tar_rv
  )
  if (NOT _tar_rv EQUAL 0)
    message (FATAL_ERROR "Failed to extract ${_archive}")
  endif ()
endif ()

if (NOT EXISTS "${_sdk_dir}/bin/clang")
  message (FATAL_ERROR "wasi-sdk clang not found at ${_sdk_dir}/bin/clang (set WASI_SDK_PATH)")
endif ()

set (_toolchain "${_sdk_dir}/share/cmake/wasi-sdk-p1.cmake")
if (NOT EXISTS "${_toolchain}")
  set (_toolchain "${_sdk_dir}/share/cmake/wasi-sdk.cmake")
endif ()
if (NOT EXISTS "${_toolchain}")
  message (FATAL_ERROR "wasi-sdk cmake toolchain not found under ${_sdk_dir}/share/cmake/")
endif ()

if (NOT CMAKE_TOOLCHAIN_FILE)
  set (CMAKE_TOOLCHAIN_FILE "${_toolchain}" CACHE FILEPATH "WASI CMake toolchain" FORCE)
endif ()
set (WASI_SDK_PATH "${_sdk_dir}" CACHE PATH "wasi-sdk prefix" FORCE)
message (STATUS "WASI SDK: ${WASI_SDK_PATH}")
message (STATUS "WASI toolchain: ${CMAKE_TOOLCHAIN_FILE}")
