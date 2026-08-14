# Resolves the project version from a git tag (e.g. 1.2.3) and writes:
#   bennugd_git.h          — VERSION / BENNUGD_VERSION for banners
#   bennugd_version.cmake  — BENNUGD_RESOLVED_VERSION for cmake --install
#   VERSION                — plain text for CI artifact names
#
# Override with -DBENNUGD_VERSION=1.2.3 (empty = detect from git).

if (NOT DEFINED GIT_DIR OR NOT DEFINED OUT)
  message (FATAL_ERROR "git_id.cmake requires -DGIT_DIR= and -DOUT=")
endif ()

if (NOT DEFINED BENNUGD_VERSION)
  set (BENNUGD_VERSION "")
endif ()

get_filename_component (OUT_DIR "${OUT}" DIRECTORY)
file (MAKE_DIRECTORY "${OUT_DIR}")

set (HASH "")
set (BRANCH "")
set (DISPLAY "")

find_program (GIT_EXECUTABLE git)
if (GIT_EXECUTABLE)
  execute_process (
    COMMAND "${GIT_EXECUTABLE}" -C "${GIT_DIR}" rev-parse --short HEAD
    OUTPUT_VARIABLE HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    RESULT_VARIABLE _git_hash_rv
  )
  if (NOT _git_hash_rv EQUAL 0)
    set (HASH "")
  endif ()

  execute_process (
    COMMAND "${GIT_EXECUTABLE}" -C "${GIT_DIR}" rev-parse --abbrev-ref HEAD
    OUTPUT_VARIABLE BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    RESULT_VARIABLE _git_branch_rv
  )
  if (NOT _git_branch_rv EQUAL 0)
    set (BRANCH "")
  endif ()

  execute_process (
    COMMAND "${GIT_EXECUTABLE}" -C "${GIT_DIR}" describe --tags --exact-match HEAD
    OUTPUT_VARIABLE _exact_tag
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    RESULT_VARIABLE _exact_rv
  )
  if (_exact_rv EQUAL 0 AND NOT _exact_tag STREQUAL "")
    set (DISPLAY "${_exact_tag}")
  else ()
    execute_process (
      COMMAND "${GIT_EXECUTABLE}" -C "${GIT_DIR}" describe --tags --always --dirty
      OUTPUT_VARIABLE _describe
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
      RESULT_VARIABLE _describe_rv
    )
    if (_describe_rv EQUAL 0 AND NOT _describe STREQUAL "")
      set (DISPLAY "${_describe}")
    endif ()
  endif ()
endif ()

# CI checkouts are often detached HEAD; GitHub Actions exposes the ref name.
if (HASH STREQUAL "" AND DEFINED ENV{GITHUB_SHA} AND NOT "$ENV{GITHUB_SHA}" STREQUAL "")
  string (SUBSTRING "$ENV{GITHUB_SHA}" 0 7 HASH)
endif ()

if (BRANCH STREQUAL "" OR BRANCH STREQUAL "HEAD")
  if (DEFINED ENV{GITHUB_HEAD_REF} AND NOT "$ENV{GITHUB_HEAD_REF}" STREQUAL "")
    set (BRANCH "$ENV{GITHUB_HEAD_REF}")
  elseif (DEFINED ENV{GITHUB_REF_NAME} AND NOT "$ENV{GITHUB_REF_NAME}" STREQUAL "")
    set (BRANCH "$ENV{GITHUB_REF_NAME}")
  endif ()
endif ()

if (DISPLAY STREQUAL "" AND DEFINED ENV{GITHUB_REF_TYPE} AND "$ENV{GITHUB_REF_TYPE}" STREQUAL "tag")
  if (DEFINED ENV{GITHUB_REF_NAME} AND NOT "$ENV{GITHUB_REF_NAME}" STREQUAL "")
    set (DISPLAY "$ENV{GITHUB_REF_NAME}")
  endif ()
endif ()

if (NOT BENNUGD_VERSION STREQUAL "")
  set (DISPLAY "${BENNUGD_VERSION}")
endif ()

if (DISPLAY STREQUAL "")
  if (NOT HASH STREQUAL "")
    set (DISPLAY "0.0.0+${HASH}")
  else ()
    set (DISPLAY "dev")
  endif ()
endif ()

set (GIT_ID "")
if (NOT BRANCH STREQUAL "" AND NOT HASH STREQUAL "")
  set (GIT_ID "${BRANCH}@${HASH}")
elseif (NOT HASH STREQUAL "")
  set (GIT_ID "${HASH}")
elseif (NOT BRANCH STREQUAL "")
  set (GIT_ID "${BRANCH}")
endif ()

# Compiler macros (__BGD__/__BGD_MINOR__/__BGD_PATCHLEVEL__) need X.Y.Z.
set (VERSION_SHORT "0.0.0")
if (DISPLAY MATCHES "^v?([0-9]+)\\.([0-9]+)\\.([0-9]+)")
  set (VERSION_SHORT "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3}")
endif ()

set (_esc_display "${DISPLAY}")
set (_esc_short "${VERSION_SHORT}")
set (_esc_git "${GIT_ID}")
foreach (_var _esc_display _esc_short _esc_git)
  string (REPLACE "\\" "\\\\" ${_var} "${${_var}}")
  string (REPLACE "\"" "\\\"" ${_var} "${${_var}}")
endforeach ()

set (_content "#ifndef BENNUGD_GIT_H
#define BENNUGD_GIT_H
#define VERSION \"${_esc_short}\"
#define BENNUGD_VERSION \"${_esc_display}\"
#define BENNUGD_GIT_ID \"${_esc_git}\"
#define BENNUGD_GIT_BANNER \"\"
#endif
")

set (_tmp "${OUT}.tmp")
file (WRITE "${_tmp}" "${_content}")
execute_process (COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${_tmp}" "${OUT}")
file (REMOVE "${_tmp}")

file (WRITE "${OUT_DIR}/bennugd_version.cmake"
  "set (BENNUGD_RESOLVED_VERSION \"${_esc_display}\")\nset (BENNUGD_VERSION_SHORT \"${_esc_short}\")\n"
)
file (WRITE "${OUT_DIR}/VERSION" "${DISPLAY}\n")
