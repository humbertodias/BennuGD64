# Resolve wasi-sdk and set CMAKE_TOOLCHAIN_FILE. Included before project() when
# BENNUGD_WASI=ON:
#   cmake -B build-wasi -DBENNUGD_WASI=ON
#   cmake --build build-wasi --target bgdc
#
# Honors WASI_SDK_PATH if it already points at a usable SDK.
# If CMAKE_TOOLCHAIN_FILE already selects a runnable wasi-sdk, skip the download.
#
# Host arch must come from `uname -m`, not CMAKE_HOST_SYSTEM_PROCESSOR: on Linux
# that variable is `uname -p`, which is often "unknown" on aarch64 Ubuntu, and
# we would then fetch the x86_64 SDK. Running that clang in an arm64 container
# fails with a missing /lib64/ld-linux-x86-64.so.2 (OrbStack/Apple Silicon).

set (BENNUGD_WASI_SDK_VERSION "33" CACHE STRING "wasi-sdk major version to download")
set (BENNUGD_WASI_SDK_VERSION_FULL "${BENNUGD_WASI_SDK_VERSION}.0" CACHE STRING "wasi-sdk full version to download")

get_filename_component (_bennu_root "${CMAKE_CURRENT_LIST_DIR}" DIRECTORY)

function (_bennugd_wasi_clang_runs clang_path out_var)
  set (_ok FALSE)
  if (EXISTS "${clang_path}")
    execute_process (
      COMMAND "${clang_path}" --version
      RESULT_VARIABLE _rv
      OUTPUT_QUIET
      ERROR_QUIET
    )
    if (_rv EQUAL 0)
      set (_ok TRUE)
    endif ()
  endif ()
  set (${out_var} "${_ok}" PARENT_SCOPE)
endfunction ()

function (_bennugd_wasi_sdk_from_toolchain toolchain_file out_var)
  get_filename_component (_cmake_dir "${toolchain_file}" DIRECTORY)
  get_filename_component (_share_dir "${_cmake_dir}" DIRECTORY)
  get_filename_component (_sdk "${_share_dir}" DIRECTORY)
  set (${out_var} "${_sdk}" PARENT_SCOPE)
endfunction ()

if (CMAKE_TOOLCHAIN_FILE AND EXISTS "${CMAKE_TOOLCHAIN_FILE}" AND CMAKE_TOOLCHAIN_FILE MATCHES "wasi")
  _bennugd_wasi_sdk_from_toolchain ("${CMAKE_TOOLCHAIN_FILE}" _cached_sdk)
  _bennugd_wasi_clang_runs ("${_cached_sdk}/bin/clang" _cached_ok)
  if (_cached_ok)
    message (STATUS "Using existing WASI toolchain: ${CMAKE_TOOLCHAIN_FILE}")
    return ()
  endif ()
  message (WARNING "Cached WASI toolchain cannot run (${_cached_sdk}/bin/clang); fetching a host SDK")
endif ()

set (_host_os "${CMAKE_HOST_SYSTEM_NAME}")
set (_host_cpu "${CMAKE_HOST_SYSTEM_PROCESSOR}")
if (NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
  execute_process (
    COMMAND uname -s
    OUTPUT_VARIABLE _uname_s
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
  execute_process (
    COMMAND uname -m
    OUTPUT_VARIABLE _uname_m
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
  if (_uname_s)
    set (_host_os "${_uname_s}")
  endif ()
  if (_uname_m)
    set (_host_cpu "${_uname_m}")
  endif ()
endif ()

if (_host_os MATCHES "Darwin")
  set (_wasi_os "macos")
elseif (_host_os MATCHES "Windows|MINGW|MSYS|CYGWIN")
  set (_wasi_os "windows")
else ()
  set (_wasi_os "linux")
endif ()

if (_host_cpu MATCHES "arm64|aarch64|ARM64")
  set (_wasi_arch "arm64")
elseif (_host_cpu MATCHES "riscv64")
  set (_wasi_arch "riscv64")
else ()
  set (_wasi_arch "x86_64")
endif ()

message (STATUS "WASI host: os=${_wasi_os} arch=${_wasi_arch} (uname ${_host_os} ${_host_cpu})")

set (_sdk_dir "")
set (_sdk_explicit FALSE)
if (DEFINED ENV{WASI_SDK_PATH} AND EXISTS "$ENV{WASI_SDK_PATH}/bin/clang")
  set (_sdk_dir "$ENV{WASI_SDK_PATH}")
  set (_sdk_explicit TRUE)
elseif (DEFINED WASI_SDK_PATH AND EXISTS "${WASI_SDK_PATH}/bin/clang")
  set (_sdk_dir "${WASI_SDK_PATH}")
  set (_sdk_explicit TRUE)
endif ()

if (_sdk_explicit)
  _bennugd_wasi_clang_runs ("${_sdk_dir}/bin/clang" _explicit_ok)
  if (NOT _explicit_ok)
    message (FATAL_ERROR
      "WASI_SDK_PATH=${_sdk_dir} has a clang that cannot run on this host "
      "(need ${_wasi_arch}-${_wasi_os}). Unset WASI_SDK_PATH to auto-download."
    )
  endif ()
endif ()

if (_sdk_dir STREQUAL "")
  set (_sdk_dir "${_bennu_root}/.deps/wasi-sdk-${BENNUGD_WASI_SDK_VERSION_FULL}-${_wasi_arch}-${_wasi_os}")
endif ()

_bennugd_wasi_clang_runs ("${_sdk_dir}/bin/clang" _sdk_ok)
if (NOT _sdk_ok)
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

_bennugd_wasi_clang_runs ("${_sdk_dir}/bin/clang" _sdk_ok)
if (NOT _sdk_ok)
  message (FATAL_ERROR
    "wasi-sdk clang not runnable at ${_sdk_dir}/bin/clang "
    "(host ${_wasi_arch}-${_wasi_os}; set WASI_SDK_PATH to a matching SDK)"
  )
endif ()

set (_toolchain "${_sdk_dir}/share/cmake/wasi-sdk-p1.cmake")
if (NOT EXISTS "${_toolchain}")
  set (_toolchain "${_sdk_dir}/share/cmake/wasi-sdk.cmake")
endif ()
if (NOT EXISTS "${_toolchain}")
  message (FATAL_ERROR "wasi-sdk cmake toolchain not found under ${_sdk_dir}/share/cmake/")
endif ()

set (CMAKE_TOOLCHAIN_FILE "${_toolchain}" CACHE FILEPATH "WASI CMake toolchain" FORCE)
set (WASI_SDK_PATH "${_sdk_dir}" CACHE PATH "wasi-sdk prefix" FORCE)
message (STATUS "WASI SDK: ${WASI_SDK_PATH}")
message (STATUS "WASI toolchain: ${CMAKE_TOOLCHAIN_FILE}")
