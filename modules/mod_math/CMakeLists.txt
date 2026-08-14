include_directories (
  ../../core/include
)

add_library (
  mod_math
  ${MODULES}
  mod_math.c
)

if (NOT WIN32)
  target_link_libraries (
    mod_math
    m
  )
endif ()
