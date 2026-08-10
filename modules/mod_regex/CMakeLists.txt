include_directories (
  ../../core/include
  ../../core/bgdrtm/include
)

# Bundled GNU regex expects autoconf feature macros.
add_definitions (-DSTDC_HEADERS -DHAVE_STRING_H)

# MSVC has no POSIX alloca in the link model used here; use malloc instead.
if (WIN32)
  add_definitions (-DREGEX_MALLOC)
endif ()

add_library (
  mod_regex
  ${MODULES}
  mod_regex.c
  regex.c
  #mod_regex.def
)

target_link_libraries (
  mod_regex
  bgdrtm
)

if (NOT STATIC_MODULES)
  install (
    TARGETS
    mod_regex
    DESTINATION
    /usr/lib
  )
endif ()
