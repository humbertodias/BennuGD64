include (CheckSymbolExists)

if (NOT LIBRETRO_COMMON_DIR)
  message (FATAL_ERROR "LIBRETRO_COMMON_DIR is not set (cmake/platforms/Libretro.cmake)")
endif ()

if ( BENNUGD_LIBRETRO_STATIC_CORE )
  set ( CORE_LIBRARY_TYPE STATIC )
else ()
  set ( CORE_LIBRARY_TYPE SHARED )
endif ()

set (_libretro_sources
  ${LIBRETRO_COMMON_DIR}/libco/libco.c
  ${LIBRETRO_COMMON_DIR}/vfs/vfs_implementation.c
  ${LIBRETRO_COMMON_DIR}/file/file_path.c
  ${LIBRETRO_COMMON_DIR}/file/file_path_io.c
  ${LIBRETRO_COMMON_DIR}/file/retro_dirent.c
  ${LIBRETRO_COMMON_DIR}/string/stdstring.c
  ${LIBRETRO_COMMON_DIR}/encodings/encoding_utf.c
  ${LIBRETRO_COMMON_DIR}/time/rtime.c
  ${LIBRETRO_COMMON_DIR}/compat/compat_strl.c
  ${LIBRETRO_COMMON_DIR}/compat/compat_strldup.c
  ${LIBRETRO_COMMON_DIR}/streams/file_stream.c
  ${LIBRETRO_COMMON_DIR}/streams/trans_stream_zlib.c
  ${LIBRETRO_COMMON_DIR}/streams/file_stream_transforms.c
  ${LIBRETRO_COMMON_DIR}/compat/compat_posix_string.c
  ${LIBRETRO_COMMON_DIR}/compat/fopen_utf8.c
  libretro.c
  filesystem.c
  filestream_gzip.c
)

if (PLATFORM_DREAMCAST OR DREAMCAST)
  list (REMOVE_ITEM _libretro_sources ${LIBRETRO_COMMON_DIR}/libco/libco.c)
  list (APPEND _libretro_sources libco_kos.c)
endif ()

# Incomplete pthread APIs (KOS, libogc gx_pthread vs pthread_t, …).
# The core is cooperative via libco; filesystem only needs a mutex stub.
if (BENNUGD_LIBRETRO_STATIC_CORE)
  list (APPEND _libretro_sources rthreads_stub.c)
  set (_libretro_have_threads 0)
else ()
  list (APPEND _libretro_sources ${LIBRETRO_COMMON_DIR}/rthreads/rthreads.c)
  set (_libretro_have_threads 1)
endif ()

add_library ( bennugd_libretro ${CORE_LIBRARY_TYPE}
  ${_libretro_sources}
)

# PSL1GHT has no Sony CELL SDK. Undefine __PS3__ so VFS uses POSIX I/O.
if (PLATFORM_PS3 OR PS3)
  set_source_files_properties (
    ${LIBRETRO_COMMON_DIR}/vfs/vfs_implementation.c
    PROPERTIES COMPILE_OPTIONS "-U__PS3__"
  )
endif ()

check_symbol_exists ( posix_memalign stdlib.h BENNUGD_LIBRETRO_HAVE_POSIX_MEMALIGN )
if ( BENNUGD_LIBRETRO_HAVE_POSIX_MEMALIGN )
  target_compile_definitions ( bennugd_libretro PRIVATE HAVE_POSIX_MEMALIGN=1 )
else ()
  target_compile_definitions ( bennugd_libretro PRIVATE HAVE_POSIX_MEMALIGN=0 )
endif ()

target_compile_definitions ( bennugd_libretro PRIVATE
  HAVE_ZLIB=1
  HAVE_THREADS=${_libretro_have_threads}
  SDL_MAIN_HANDLED=1
)

target_include_directories ( bennugd_libretro PRIVATE
  ${LIBRETRO_COMMON_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/core/include
  ${CMAKE_SOURCE_DIR}/core/bgdi/include
  ${CMAKE_SOURCE_DIR}/core/bgdrtm/include
  ${CMAKE_SOURCE_DIR}/modules/libvideo
  ${CMAKE_SOURCE_DIR}/modules/libgrbase
  ${CMAKE_SOURCE_DIR}/modules/mod_sound
)

if ( CMAKE_C_COMPILER_ID MATCHES "MSVC" )
  set_source_files_properties ( ${LIBRETRO_COMMON_DIR}/libco/libco.c PROPERTIES COMPILE_OPTIONS "/wd4005" )
endif ()

set_target_properties ( bennugd_libretro PROPERTIES
  PREFIX ""
  OUTPUT_NAME "bennugd_libretro"
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}
)

if ( UNIX AND NOT APPLE AND NOT BENNUGD_LIBRETRO_STATIC_CORE )
  target_link_options ( bennugd_libretro PRIVATE -Wl,-Bsymbolic )
endif ()

target_link_libraries ( bennugd_libretro PRIVATE
  bgdi
  ${BENNUGD_SDL3_TARGET}
  ${BENNUGD_SDL3_MIXER_TARGET}
  ZLIB::ZLIB
  PNG::PNG
)

if ( UNIX AND NOT APPLE AND NOT ANDROID AND NOT BENNUGD_LIBRETRO_STATIC_CORE )
  target_link_libraries ( bennugd_libretro PRIVATE m pthread )
elseif ( ANDROID )
  target_link_libraries ( bennugd_libretro PRIVATE android log m )
endif ()

install ( TARGETS bennugd_libretro
  LIBRARY DESTINATION .
  RUNTIME DESTINATION .
  ARCHIVE DESTINATION .
)
install ( FILES bennugd_libretro.info DESTINATION . OPTIONAL )
