# I/O backend for core/common/files.c: files_$platform.c

function (bennugd_files_native target)
  if (NINTENDO_WII OR PLATFORM_WII)
    target_sources (${target} PRIVATE ${CMAKE_SOURCE_DIR}/core/common/files_wii.c)
  else ()
    target_sources (${target} PRIVATE ${CMAKE_SOURCE_DIR}/core/common/files_native.c)
  endif ()
endfunction ()
