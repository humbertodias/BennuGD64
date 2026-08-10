include_directories (
  ../../core/include
  ../../core/bgdrtm/include
)

# Bundled GNU regex expects autoconf feature macros.
add_definitions (-DSTDC_HEADERS -DHAVE_STRING_H)

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
