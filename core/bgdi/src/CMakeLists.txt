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
if (PLATFORM_PS2 OR PS2)
  set (_ps2sdk "$ENV{PS2SDK}")
  if (NOT _ps2sdk)
    set (_ps2sdk "$ENV{PS2DEV}/ps2sdk")
  endif ()
  set (_bin2c "")
  foreach ( _cand "${_ps2sdk}/bin/bin2c" "$ENV{PS2DEV}/bin/bin2c")
    if (EXISTS "${_cand}")
      set (_bin2c "${_cand}")
      break ()
    endif ()
  endforeach ()
  if (_bin2c STREQUAL "")
    message (FATAL_ERROR "bin2c not found (need USB IRX embedding)")
  endif ()
  set (_usbd "")
  set (_usbmass "")
  foreach ( _cand
      "${_ps2sdk}/iop/irx/usbd.irx"
      "${_ps2sdk}/ee/lib/usbd.irx"
  )
    if (EXISTS "${_cand}")
      set (_usbd "${_cand}")
      break ()
    endif ()
  endforeach ()
  foreach ( _cand
      "${_ps2sdk}/iop/irx/usbhdfsd.irx"
      "${_ps2sdk}/ee/lib/usbhdfsd.irx"
  )
    if (EXISTS "${_cand}")
      set (_usbmass "${_cand}")
      break ()
    endif ()
  endforeach ()
  if (_usbd STREQUAL "" OR _usbmass STREQUAL "")
    message (FATAL_ERROR "usbd.irx / usbhdfsd.irx not found under ${_ps2sdk}")
  endif ()
  add_custom_command (
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/usbd_irx.c"
    COMMAND "${_bin2c}" "${_usbd}" "${CMAKE_CURRENT_BINARY_DIR}/usbd_irx.c" usbd_irx
    DEPENDS "${_usbd}"
  )
  add_custom_command (
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/usb_mass_irx.c"
    COMMAND "${_bin2c}" "${_usbmass}" "${CMAKE_CURRENT_BINARY_DIR}/usb_mass_irx.c" usb_mass_irx
    DEPENDS "${_usbmass}"
  )
  target_sources (bgdi PRIVATE
    main_ps2.c
    "${CMAKE_CURRENT_BINARY_DIR}/usbd_irx.c"
    "${CMAKE_CURRENT_BINARY_DIR}/usb_mass_irx.c"
  )
  set_source_files_properties (
    "${CMAKE_CURRENT_BINARY_DIR}/usbd_irx.c"
    "${CMAKE_CURRENT_BINARY_DIR}/usb_mass_irx.c"
    PROPERTIES GENERATED TRUE
  )
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
if (NINTENDO_WII OR PLATFORM_WII)
  target_sources (bgdi PRIVATE main_wii.c main_wii_posix.c)
endif ()
if (WIN32)
  target_sources (bgdi PRIVATE main_win32.c)
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

if (PLATFORM_PS2 OR PS2)
  target_link_libraries (
    bgdi
    ${BENNUGD_SDL3_TARGET}
    ${BENNUGD_SDL3_MIXER_TARGET}
    "-Wl,--start-group"
    gskit
    dmakit
    patches
    pad
    audsrv
    cdvd
    -ldebug
    ps2_drivers
    m
    "-Wl,--end-group"
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

if (NINTENDO_WII OR PLATFORM_WII)
  target_link_libraries (
    bgdi
    ${BENNUGD_SDL3_TARGET}
    "-Wl,--start-group"
    wiiuse
    bte
    wiikeyboard
    aesnd
    fat
    ogc
    m
    "-Wl,--end-group"
  )
endif ()

if (EMSCRIPTEN)
  bennugd_emscripten_link (bgdi)
endif ()

if (TARGET bennugd_osxcross_rt)
  target_link_libraries (bgdi bennugd_osxcross_rt)
endif ()
