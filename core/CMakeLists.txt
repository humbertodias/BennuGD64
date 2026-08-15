if (NOT COMPILER_ONLY)
  add_subdirectory (bgdrtm)
  add_subdirectory (bgdi)
endif ()

if (NOT INTERPRETER_ONLY)
  add_subdirectory (bgdc)
endif ()
