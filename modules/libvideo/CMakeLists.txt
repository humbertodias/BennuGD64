set (BENNUGD_GIT_HEADER "${CMAKE_BINARY_DIR}/generated/bennugd_git.h")
execute_process (
  COMMAND "${CMAKE_COMMAND}"
    -DGIT_DIR=${CMAKE_SOURCE_DIR}
    -DOUT=${BENNUGD_GIT_HEADER}
    -P ${CMAKE_SOURCE_DIR}/cmake/git_id.cmake
)
add_custom_target (
  bennugd_git_header
  COMMAND "${CMAKE_COMMAND}"
    -DGIT_DIR=${CMAKE_SOURCE_DIR}
    -DOUT=${BENNUGD_GIT_HEADER}
    -P ${CMAKE_SOURCE_DIR}/cmake/git_id.cmake
  BYPRODUCTS ${BENNUGD_GIT_HEADER}
  COMMENT "Updating git id for window title"
)

include_directories (
  ../../core/include
  ../../core/bgdrtm/include
  ../libgrbase
  ${CMAKE_BINARY_DIR}/generated
)

add_library (
  video
  ${MODULES}
  g_video.c
  g_regions.c
) 

add_dependencies ( video bennugd_git_header )

target_link_libraries (
  video
  bgdrtm
  grbase
  ${BENNUGD_SDL3_TARGET}
)

if (NOT STATIC_MODULES)
  install (
    TARGETS
    video
    DESTINATION
    /usr/lib
  )
endif ()
