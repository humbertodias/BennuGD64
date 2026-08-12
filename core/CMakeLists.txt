add_subdirectory (bgdrtm)
add_subdirectory (bgdi)

if(NOT INTERPRETER_ONLY)
  add_subdirectory (bgdc)
endif ()
