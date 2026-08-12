include_directories (
  ../../core/include
  ../libgrbase
)

add_library (
  mod_mem
  ${MODULES}
  mod_mem.c
)

target_link_libraries (
  mod_mem
  grbase
)

if (NOT STATIC_MODULES)
  install (
    TARGETS
    mod_mem
    DESTINATION
    /usr/lib
  )
endif ()
