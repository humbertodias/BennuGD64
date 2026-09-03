# wasm32 / Emscripten: static interpreter only (no dlopen, no OpenSSL).

set (STATIC_MODULES ON CACHE BOOL "WASM cannot load shared Bennu modules" FORCE)

if (NOT USE_LIBDES)
  message (WARNING "Emscripten builds should use -DUSE_LIBDES=ON (OpenSSL is not used on the web)")
endif ()

function (bennugd_emscripten_link target)
  target_link_options (${target} PRIVATE
    -sALLOW_MEMORY_GROWTH=1
    -sINITIAL_MEMORY=67108864
    -sSTACK_SIZE=262144
    -sASYNCIFY=1
    -sASYNCIFY_STACK_SIZE=524288
    "SHELL:-sASYNCIFY_ADD=['instance_go_all','bgdrtm_frame_throttle']"
    -sFORCE_FILESYSTEM=1
    -sEXIT_RUNTIME=0
    -sINVOKE_RUN=0
    -sENVIRONMENT=web
    -sMIN_WEBGL_VERSION=1
    -sMAX_WEBGL_VERSION=2
    "SHELL:-sEXPORTED_RUNTIME_METHODS=['FS','callMain']"
    "SHELL:--shell-file ${CMAKE_SOURCE_DIR}/platforms/web/shell.html"
  )
  set_target_properties (${target} PROPERTIES SUFFIX ".html")
  set_property (TARGET ${target} APPEND PROPERTY
    LINK_DEPENDS "${CMAKE_SOURCE_DIR}/platforms/web/shell.html"
  )

  file (GLOB _demo_dcbs CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/platforms/web/demo/*.dcb")
  foreach (_demo_dcb IN LISTS _demo_dcbs)
    get_filename_component (_demo_name "${_demo_dcb}" NAME)
    target_link_options (${target} PRIVATE
      "SHELL:--preload-file ${_demo_dcb}@demo/${_demo_name}"
    )
  endforeach ()
endfunction ()
