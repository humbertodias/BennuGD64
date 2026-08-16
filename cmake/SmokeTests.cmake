# CTest smokes: banner text and compiling web/demo/hello.prg.
# Cross-compiled Windows binaries cannot run on the Linux builder.

enable_testing ()

if (NOT TARGET bgdc)
  return ()
endif ()

set (_smoke_dir "${CMAKE_SOURCE_DIR}/cmake")
set (_hello_prg "${CMAKE_SOURCE_DIR}/web/demo/hello.prg")
set (_hello_dcb "${CMAKE_BINARY_DIR}/smoke-hello.dcb")

set (_can_run_native TRUE)
if (CMAKE_CROSSCOMPILING AND NOT CMAKE_SYSTEM_NAME MATCHES "WASI")
  set (_can_run_native FALSE)
endif ()

if (CMAKE_SYSTEM_NAME MATCHES "WASI")
  find_program (WASMTIME_EXECUTABLE wasmtime)
  if (WASMTIME_EXECUTABLE)
    add_test (
      NAME smoke.bgdc.help
      COMMAND "${CMAKE_COMMAND}"
        -DBINARY=$<TARGET_FILE:bgdc>
        -DPATTERN=BGDC|Compiler
        -DWASMTIME=${WASMTIME_EXECUTABLE}
        -P "${_smoke_dir}/smoke_banner.cmake"
    )
    add_test (
      NAME smoke.bgdc.hello
      COMMAND "${CMAKE_COMMAND}"
        -DBGDC=$<TARGET_FILE:bgdc>
        -DPRG=${_hello_prg}
        -DOUT=${_hello_dcb}
        -DWASMTIME=${WASMTIME_EXECUTABLE}
        -DBINARY_DIR=${CMAKE_BINARY_DIR}
        -P "${_smoke_dir}/smoke_compile.cmake"
    )
  endif ()
elseif (_can_run_native)
  add_test (
    NAME smoke.bgdc.help
    COMMAND "${CMAKE_COMMAND}"
      -DBINARY=$<TARGET_FILE:bgdc>
      -DPATTERN=BGDC|Compiler
      -P "${_smoke_dir}/smoke_banner.cmake"
  )
  if (TARGET bgdi)
    add_test (
      NAME smoke.bgdi.help
      COMMAND "${CMAKE_COMMAND}"
        -DBINARY=$<TARGET_FILE:bgdi>
        -DPATTERN=BGDI|Interpreter
        -P "${_smoke_dir}/smoke_banner.cmake"
    )
  endif ()
  # Shared builds load plugins from <appexepath>/modules/; that layout exists
  # after cmake --install, not in the build tree.
  if (STATIC_MODULES)
    add_test (
      NAME smoke.bgdc.hello
      COMMAND "${CMAKE_COMMAND}"
        -DBGDC=$<TARGET_FILE:bgdc>
        -DPRG=${_hello_prg}
        -DOUT=${_hello_dcb}
        -P "${_smoke_dir}/smoke_compile.cmake"
    )
  endif ()
endif ()
