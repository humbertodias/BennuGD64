include_directories (
  ../../core/include
  ../libgrbase
)

add_library (
  mod_effects
  ${MODULES}
  mod_effects.c
) 

target_link_libraries (
  mod_effects
  grbase
)
