# Writes bennugd_git.h with BENNUGD_GIT_ID "branch@commit" for the window title.
# Invoked at configure time and before compiling libvideo.

if (NOT DEFINED GIT_DIR OR NOT DEFINED OUT)
  message(FATAL_ERROR "git_id.cmake requires -DGIT_DIR= and -DOUT=")
endif ()

get_filename_component (OUT_DIR "${OUT}" DIRECTORY)
file (MAKE_DIRECTORY "${OUT_DIR}")

set (HASH "")
set (BRANCH "")

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

set (GIT_ID "")
if (NOT BRANCH STREQUAL "" AND NOT HASH STREQUAL "")
  set (GIT_ID "${BRANCH}@${HASH}")
elseif (NOT HASH STREQUAL "")
  set (GIT_ID "${HASH}")
elseif (NOT BRANCH STREQUAL "")
  set (GIT_ID "${BRANCH}")
endif ()

string (REPLACE "\\" "\\\\" GIT_ID "${GIT_ID}")
string (REPLACE "\"" "\\\"" GIT_ID "${GIT_ID}")

set (_content "#ifndef BENNUGD_GIT_H\n#define BENNUGD_GIT_H\n#define BENNUGD_GIT_ID \"${GIT_ID}\"\n#endif\n")
set (_tmp "${OUT}.tmp")
file (WRITE "${_tmp}" "${_content}")
execute_process (COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${_tmp}" "${OUT}")
file (REMOVE "${_tmp}")
