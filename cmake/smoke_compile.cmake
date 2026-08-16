# cmake -P helper: compile web/demo/hello.prg to a non-empty .dcb.
# Native:  -DBGDC=path -DPRG=hello.prg -DOUT=hello.dcb
# WASI:    also -DWASMTIME=wasmtime (copies wasm + prg into a temp dir)

if (NOT DEFINED BGDC OR NOT DEFINED PRG OR NOT DEFINED OUT)
  message (FATAL_ERROR "smoke_compile.cmake needs -DBGDC= -DPRG= -DOUT=")
endif ()

get_filename_component (_outdir "${OUT}" DIRECTORY)
if (_outdir)
  file (MAKE_DIRECTORY "${_outdir}")
endif ()

if (DEFINED WASMTIME AND NOT WASMTIME STREQUAL "")
  string (RANDOM _id)
  set (_work "${CMAKE_CURRENT_BINARY_DIR}/smoke-wasi-${_id}")
  if (DEFINED BINARY_DIR AND NOT BINARY_DIR STREQUAL "")
    set (_work "${BINARY_DIR}/smoke-wasi-${_id}")
  endif ()
  file (MAKE_DIRECTORY "${_work}")
  file (COPY "${BGDC}" DESTINATION "${_work}")
  file (COPY "${PRG}" DESTINATION "${_work}")
  get_filename_component (_wasm_name "${BGDC}" NAME)
  get_filename_component (_prg_name "${PRG}" NAME)
  execute_process (
    COMMAND "${WASMTIME}" --dir=. "./${_wasm_name}" -- -o hello.dcb "${_prg_name}"
    WORKING_DIRECTORY "${_work}"
    RESULT_VARIABLE _rv
    OUTPUT_VARIABLE _out
    ERROR_VARIABLE _err
  )
  message (STATUS "${_out}${_err}")
  if (NOT _rv EQUAL 0)
    file (REMOVE_RECURSE "${_work}")
    message (FATAL_ERROR "wasmtime/bgdc failed (exit ${_rv})")
  endif ()
  if (NOT EXISTS "${_work}/hello.dcb")
    file (REMOVE_RECURSE "${_work}")
    message (FATAL_ERROR "hello.dcb was not produced")
  endif ()
  execute_process (COMMAND "${CMAKE_COMMAND}" -E copy "${_work}/hello.dcb" "${OUT}")
  file (REMOVE_RECURSE "${_work}")
else ()
  execute_process (
    COMMAND "${BGDC}" -o "${OUT}" "${PRG}"
    RESULT_VARIABLE _rv
    OUTPUT_VARIABLE _out
    ERROR_VARIABLE _err
  )
  message (STATUS "${_out}${_err}")
  if (NOT _rv EQUAL 0)
    message (FATAL_ERROR "bgdc failed (exit ${_rv})")
  endif ()
endif ()

if (NOT EXISTS "${OUT}")
  message (FATAL_ERROR "missing ${OUT}")
endif ()
file (SIZE "${OUT}" _sz)
if (_sz EQUAL 0)
  message (FATAL_ERROR "${OUT} is empty")
endif ()
message (STATUS "hello.dcb ${_sz} bytes")
