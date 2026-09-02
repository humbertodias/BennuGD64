add_definitions (-D__BGDRTM__)
add_definitions (-DDEBUG)

include_directories (
  ../include
  ../../include
)

if (USE_LIBDES)
  include_directories (
    ../../3rdparty/des-4.04b
  )
endif ()

add_library (
  bgdrtm
  ${MODULES}
  fmath.c
  dcbr.c
  sysprocs.c
  strings.c
  dirs.c
  copy.c
  misc.c
  instance.c
  interpreter.c
  frame_pace.c
  varspace_file.c
  bgd_lowmem.c
  ../../common/debug.c
  ../../common/b_crypt.c
  ../../common/files.c
  ../../common/xctype.c
)

if (NINTENDO_SWITCH OR PLATFORM_DREAMCAST OR DREAMCAST OR NINTENDO_WII OR PLATFORM_WII OR PLATFORM_PS2 OR PS2 OR PLATFORM_VITA OR VITA OR PLATFORM_PS3 OR PS3)
  target_sources (bgdrtm PRIVATE nx_glob.c)
endif ()

if (NINTENDO_WII OR PLATFORM_WII)
  target_sources (bgdrtm PRIVATE dirs_wii.c)
else ()
  target_sources (bgdrtm PRIVATE dirs_native.c)
endif ()

include (${CMAKE_SOURCE_DIR}/cmake/FilesNative.cmake)
bennugd_files_native (bgdrtm)

if (PLATFORM_PSP OR PSP)
  target_sources (bgdrtm PRIVATE misc_psp.c)
endif ()
if (PLATFORM_VITA OR VITA)
  target_sources (bgdrtm PRIVATE misc_vita.c)
endif ()
if (CMAKE_SYSTEM_NAME STREQUAL "tvOS")
  target_sources (bgdrtm PRIVATE misc_tvos.c)
endif ()
if (CMAKE_SYSTEM_NAME STREQUAL "iOS")
  target_sources (bgdrtm PRIVATE misc_ios.c)
endif ()
if (PLATFORM_PS2 OR PS2)
  target_sources (bgdrtm PRIVATE misc_ps2.c dirs_ps2.c)
endif ()
if (PLATFORM_PS3 OR PS3)
  target_sources (bgdrtm PRIVATE misc_ps3.c)
endif ()
if (EMSCRIPTEN)
  target_sources (bgdrtm PRIVATE
    misc_emscripten.c
    frame_pace_emscripten.c
    interpreter_emscripten.c
  )
endif ()

if (USE_LIBDES)
  target_link_libraries (
    bgdrtm
    ZLIB::ZLIB
    des  
  )
else ()
  target_link_libraries (
    bgdrtm
    ZLIB::ZLIB
    OpenSSL::Crypto
  )
endif ()

if (UNIX OR NINTENDO_SWITCH OR PLATFORM_DREAMCAST OR DREAMCAST OR PLATFORM_PSP OR PSP OR PLATFORM_VITA OR VITA OR PLATFORM_PS2 OR PS2 OR PLATFORM_PS3 OR PS3 OR NINTENDO_WII OR PLATFORM_WII OR CMAKE_SYSTEM_NAME STREQUAL "tvOS" OR CMAKE_SYSTEM_NAME STREQUAL "iOS")
  target_link_libraries ( bgdrtm m )
endif ()
if (PLATFORM_PS2 OR PS2)
  target_link_libraries ( bgdrtm ${BENNUGD_SDL3_TARGET} )
endif ()
if (NOT STATIC_MODULES)
  target_link_libraries ( bgdrtm ${CMAKE_DL_LIBS} )
endif ()

if (STATIC_MODULES)
  target_link_libraries (
    bgdrtm
    grbase
    bgload
    blit
    draw
    video
    render
    font
    text
    sdlhandler
    wm
    key
    mouse
    joy
    scroll
    mod_scroll
    mod_mouse
    mod_map
    mod_video
    mod_screen
    mod_blendop
    mod_text
    mod_draw
    mod_grproc
    mod_effects
    mod_key
    mod_cd
    mod_dir
    mod_file
    mod_crypt
    mod_joy
    mod_math
    mod_mem
    mod_proc
    mod_rand
    mod_regex
    mod_say
    mod_sort
    mod_sound
    mod_string
    mod_sys
    mod_time
    mod_timers
    mod_path
    mod_wm
    mod_debug
    mod_flic
    mod_m7
    mod_wpad
  )
endif ()
