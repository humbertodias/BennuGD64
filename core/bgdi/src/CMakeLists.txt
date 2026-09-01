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
if (PLATFORM_VITA OR VITA)
  target_sources (bgdi PRIVATE main_vita.c)
endif ()
if (CMAKE_SYSTEM_NAME STREQUAL "tvOS")
  target_sources (bgdi PRIVATE main_tvos.c)
endif ()
if (CMAKE_SYSTEM_NAME STREQUAL "iOS")
  target_sources (bgdi PRIVATE main_ios.c)
endif ()
if (PLATFORM_PS3 OR PS3)
  target_sources (bgdi PRIVATE main_ps3.c)
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

if (PLATFORM_VITA OR VITA)
  target_link_libraries (
    bgdi
    ${BENNUGD_SDL3_TARGET}
    ${BENNUGD_SDL3_MIXER_TARGET}
    ScePower_stub
    SceCtrl_stub
    m
  )
  if (COMMAND vita_create_self)
    vita_create_self (bgdi.self bgdi UNSAFE)
    vita_create_vpk (bennugd64.vpk BGDV00001 bgdi.self
      VERSION 01.00
      NAME BennuGD64
    )
  endif ()
endif ()

if (CMAKE_SYSTEM_NAME STREQUAL "tvOS")
  target_link_libraries (
    bgdi
    ${BENNUGD_SDL3_TARGET}
    ${BENNUGD_SDL3_MIXER_TARGET}
  )
  set_target_properties (bgdi PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER "${BENNUGD_BUNDLE_IDENTIFIER}"
    MACOSX_BUNDLE_BUNDLE_VERSION "${BENNUGD_PACKAGE_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${BENNUGD_PACKAGE_VERSION}"
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/tvos/Info.plist"
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${BENNUGD_BUNDLE_IDENTIFIER}"
    XCODE_ATTRIBUTE_BENNUGD_BUNDLE_DISPLAY_NAME "${BENNUGD_BUNDLE_NAME}"
    XCODE_ATTRIBUTE_MARKETING_VERSION "${BENNUGD_PACKAGE_VERSION}"
    XCODE_ATTRIBUTE_CURRENT_PROJECT_VERSION "1"
    XCODE_ATTRIBUTE_SKIP_INSTALL "NO"
    XCODE_ATTRIBUTE_INSTALL_PATH "$(LOCAL_APPS_DIR)"
    XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym"
    XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "3"
  )
  # actool for appletvsimulator needs a tvOS Simulator runtime (often not
  # installed). Device SDK compiles the layered App Icon without that runtime.
  if (NOT CMAKE_OSX_SYSROOT MATCHES "[Ss]imulator")
    target_sources (bgdi PRIVATE
      ${CMAKE_SOURCE_DIR}/tvos/Assets.xcassets
    )
    set_source_files_properties (
      ${CMAKE_SOURCE_DIR}/tvos/Assets.xcassets
      PROPERTIES MACOSX_PACKAGE_LOCATION Resources
    )
    set_target_properties (bgdi PROPERTIES
      XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon"
    )
  endif ()
  if (DEFINED BENNUGD_DEVELOPMENT_TEAM AND NOT BENNUGD_DEVELOPMENT_TEAM STREQUAL "")
    set_target_properties (bgdi PROPERTIES
      XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${BENNUGD_DEVELOPMENT_TEAM}"
      XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Apple Development"
      XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Automatic"
    )
  elseif (CMAKE_OSX_SYSROOT MATCHES "[Ss]imulator")
    set_target_properties (bgdi PROPERTIES
      XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO"
      XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
    )
  endif ()
  add_custom_command (TARGET bgdi POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${CMAKE_SOURCE_DIR}/tvos/contents"
      "$<TARGET_BUNDLE_DIR:bgdi>"
    COMMAND ${CMAKE_COMMAND} -E remove -f
      "$<TARGET_BUNDLE_DIR:bgdi>/.gitkeep"
    COMMENT "Copy tvos/contents into bgdi.app"
  )
endif ()

if (CMAKE_SYSTEM_NAME STREQUAL "iOS")
  target_link_libraries (
    bgdi
    ${BENNUGD_SDL3_TARGET}
    ${BENNUGD_SDL3_MIXER_TARGET}
  )
  set_target_properties (bgdi PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER "${BENNUGD_BUNDLE_IDENTIFIER}"
    MACOSX_BUNDLE_BUNDLE_VERSION "${BENNUGD_PACKAGE_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${BENNUGD_PACKAGE_VERSION}"
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/ios/Info.plist"
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${BENNUGD_BUNDLE_IDENTIFIER}"
    XCODE_ATTRIBUTE_BENNUGD_BUNDLE_DISPLAY_NAME "${BENNUGD_BUNDLE_NAME}"
    XCODE_ATTRIBUTE_MARKETING_VERSION "${BENNUGD_PACKAGE_VERSION}"
    XCODE_ATTRIBUTE_CURRENT_PROJECT_VERSION "1"
    XCODE_ATTRIBUTE_SKIP_INSTALL "NO"
    XCODE_ATTRIBUTE_INSTALL_PATH "$(LOCAL_APPS_DIR)"
    XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym"
    XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
    XCODE_ATTRIBUTE_INFOPLIST_KEY_UILaunchScreen_Generation "YES"
  )
  target_sources (bgdi PRIVATE
    ${CMAKE_SOURCE_DIR}/ios/Assets.xcassets
  )
  set_source_files_properties (
    ${CMAKE_SOURCE_DIR}/ios/Assets.xcassets
    PROPERTIES MACOSX_PACKAGE_LOCATION Resources
  )
  set_target_properties (bgdi PROPERTIES
    XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon"
  )
  if (DEFINED BENNUGD_DEVELOPMENT_TEAM AND NOT BENNUGD_DEVELOPMENT_TEAM STREQUAL "")
    set_target_properties (bgdi PROPERTIES
      XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${BENNUGD_DEVELOPMENT_TEAM}"
      XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Apple Development"
      XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Automatic"
    )
  elseif (CMAKE_OSX_SYSROOT MATCHES "[Ss]imulator")
    set_target_properties (bgdi PROPERTIES
      XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO"
      XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
    )
  endif ()
  add_custom_command (TARGET bgdi POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${CMAKE_SOURCE_DIR}/ios/contents"
      "$<TARGET_BUNDLE_DIR:bgdi>"
    COMMAND ${CMAKE_COMMAND} -E remove -f
      "$<TARGET_BUNDLE_DIR:bgdi>/.gitkeep"
    COMMENT "Copy ios/contents into bgdi.app"
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

if (PLATFORM_PS3 OR PS3)
  target_link_libraries (
    bgdi
    ${BENNUGD_SDL3_TARGET}
    ${BENNUGD_SDL3_MIXER_TARGET}
    "-Wl,--start-group"
    gcm_sys
    rsx
    sysutil
    io
    audio
    rt
    lv2
    m
    z
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
