include_directories (
  ../../core/include
)

add_library (
  mod_mathi
  ${MODULES}
  mod_mathi.c
)

if (NOT WIN32)
  target_link_libraries (
    mod_mathi
    m
  )
endif ()
