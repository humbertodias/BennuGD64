add_definitions (-D__BGDI__)
add_definitions (-DDEBUG)

include_directories (
  ../include
  ../../include
  ../../bgdrtm/include
)

if (ANDROID)
  add_library ( bgdi SHARED main.c )
  set_target_properties ( bgdi PROPERTIES OUTPUT_NAME "main" )
else ()
  add_executable (
    bgdi
    main.c
  )
endif ()

if (PLATFORM_PSP OR PSP)
  target_sources (bgdi PRIVATE main_psp.c)
endif ()
if (ANDROID)
  target_sources (bgdi PRIVATE main_android.c)
endif ()
if (NINTENDO_SWITCH)
  target_sources (bgdi PRIVATE main_switch.c)
endif ()
if (PLATFORM_DREAMCAST OR DREAMCAST)
  target_sources (bgdi PRIVATE main_dc.c)
endif ()
if (PLATFORM_PANDORA OR OPENPANDORA)
  target_sources (bgdi PRIVATE main_pandora.c)
endif ()

add_dependencies ( bgdi bennugd_git_header )

if (USE_LIBDES)
  target_link_libraries (
    bgdi
    bgdrtm
    ZLIB::ZLIB
    des
  )
else ()
  target_link_libraries (
    bgdi
    bgdrtm
    ZLIB::ZLIB
    OpenSSL::Crypto
  )
endif ()

if (ANDROID)
  target_link_libraries (
    bgdi
    ${BENNUGD_SDL3_TARGET}
    android
    log
  )
  target_link_options ( bgdi PRIVATE "-Wl,--undefined=SDL_main" )
endif ()

if (NINTENDO_SWITCH)
  target_link_libraries (
    bgdi
    ${BENNUGD_SDL3_TARGET}
    "-Wl,--start-group"
    EGL
    glapi
    drm_nouveau
    nx
    m
    stdc++
    "-Wl,--end-group"
  )
endif ()

if (PLATFORM_DREAMCAST OR DREAMCAST)
  target_link_libraries (
    bgdi
    ${BENNUGD_SDL3_TARGET}
    m
    pthread
    GL
  )
endif ()

if (PLATFORM_PSP OR PSP)
  target_link_libraries (
    bgdi
    ${BENNUGD_SDL3_TARGET}
    ${BENNUGD_SDL3_MIXER_TARGET}
    psppower
    pspdebug
    pspsdk
    c
    pspuser
    pspkernel
    m
  )
endif ()

if (PLATFORM_PANDORA OR OPENPANDORA)
  target_link_libraries (
    bgdi
    ${BENNUGD_SDL3_TARGET}
    pthread
    dl
    m
    rt
    X11
    Xext
    Xrandr
    asound
    atomic
  )
endif ()

if (EMSCRIPTEN)
  bennugd_emscripten_link (bgdi)
endif ()
