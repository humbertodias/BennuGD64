# cmake -P helper: require BINARY output to match PATTERN (regex).
# Optional WASMTIME: run `wasmtime --dir=. BINARY --` instead.

if (NOT DEFINED BINARY OR NOT DEFINED PATTERN)
  message (FATAL_ERROR "smoke_banner.cmake needs -DBINARY= and -DPATTERN=")
endif ()

if (DEFINED WASMTIME AND NOT WASMTIME STREQUAL "")
  execute_process (
    COMMAND "${WASMTIME}" --dir=. "${BINARY}" --
    RESULT_VARIABLE _rv
    OUTPUT_VARIABLE _out
    ERROR_VARIABLE _err
  )
else ()
  execute_process (
    COMMAND "${BINARY}"
    RESULT_VARIABLE _rv
    OUTPUT_VARIABLE _out
    ERROR_VARIABLE _err
  )
endif ()

set (_text "${_out}${_err}")
message (STATUS "${_text}")
if (NOT _text MATCHES "${PATTERN}")
  message (FATAL_ERROR "output did not match /${PATTERN}/ (exit ${_rv})")
endif ()
