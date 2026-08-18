# Download and build zlib, libpng, SDL3 and SDL3_mixer as static PIC libraries.
# Used when BENNUGD_BUNDLE_DEPS=ON so a single CMake configure/build is enough.

include (FetchContent)

if (POLICY CMP0135)
  cmake_policy (SET CMP0135 NEW)
endif ()

set (BENNUGD_ZLIB_VERSION "1.3.1" CACHE STRING "zlib version to fetch")
set (BENNUGD_LIBPNG_VERSION "1.6.47" CACHE STRING "libpng version to fetch")
set (BENNUGD_SDL3_REF "release-3.4.14" CACHE STRING "SDL3 git tag/branch to fetch")
set (BENNUGD_SDL3_GIT_REPOSITORY "https://github.com/libsdl-org/SDL.git" CACHE STRING "SDL3 git repository")
set (BENNUGD_SDL3_SWITCH_REF "switch-sdl-3.4" CACHE STRING "devkitPro SDL3 Switch branch")
set (BENNUGD_SDL3_DREAMCAST_REF "dreamcastSDL3" CACHE STRING "GPF SDL3 Dreamcast branch")
set (BENNUGD_SDL3_MIXER_REF "release-3.2.4" CACHE STRING "SDL3_mixer git tag/branch to fetch")

if (NINTENDO_SWITCH)
  set (BENNUGD_SDL3_GIT_REPOSITORY "https://github.com/devkitPro/SDL.git")
  set (BENNUGD_SDL3_REF "${BENNUGD_SDL3_SWITCH_REF}")
endif ()
if (PLATFORM_DREAMCAST OR DREAMCAST)
  set (BENNUGD_SDL3_GIT_REPOSITORY "https://github.com/GPF/SDL.git")
  set (BENNUGD_SDL3_REF "${BENNUGD_SDL3_DREAMCAST_REF}")
endif ()

# Static archives must be PIC so they can later link into .so/.dylib modules.
# Switch/Dreamcast/PSP/Pandora homebrew uses the toolchain PIE/KOS/pspdev flags instead.
if (NOT EMSCRIPTEN AND NOT CMAKE_SYSTEM_NAME MATCHES "WASI" AND NOT NINTENDO_SWITCH AND NOT PLATFORM_DREAMCAST AND NOT DREAMCAST AND NOT PLATFORM_PSP AND NOT PSP AND NOT PLATFORM_PANDORA AND NOT OPENPANDORA)
  set (CMAKE_POSITION_INDEPENDENT_CODE ON)
  if (NOT MSVC)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  endif ()
endif ()

set (_bennugd_saved_shared "${BUILD_SHARED_LIBS}")
set (BUILD_SHARED_LIBS OFF)

# Fetched deps are linked in-tree; do not generate their install/export rules.
# libpng's install(EXPORT) otherwise requires zlibstatic in an export set.
set (_bennugd_saved_skip_install "${CMAKE_SKIP_INSTALL_RULES}")
set (CMAKE_SKIP_INSTALL_RULES ON)
set (SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE)
set (SKIP_INSTALL_EXPORT ON CACHE BOOL "" FORCE)
set (SKIP_INSTALL_LIBRARIES ON CACHE BOOL "" FORCE)
set (SKIP_INSTALL_HEADERS ON CACHE BOOL "" FORCE)
set (SKIP_INSTALL_FILES ON CACHE BOOL "" FORCE)

# --- zlib ---
set (ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare (
  zlib
  URL "https://github.com/madler/zlib/releases/download/v${BENNUGD_ZLIB_VERSION}/zlib-${BENNUGD_ZLIB_VERSION}.tar.gz"
)
FetchContent_GetProperties (zlib)
if (NOT zlib_POPULATED)
  FetchContent_Populate (zlib)
  add_subdirectory (${zlib_SOURCE_DIR} ${zlib_BINARY_DIR} EXCLUDE_FROM_ALL)
endif ()

if (NOT TARGET ZLIB::ZLIB)
  if (TARGET zlibstatic)
    add_library (ZLIB::ZLIB ALIAS zlibstatic)
  elseif (TARGET zlib)
    add_library (ZLIB::ZLIB ALIAS zlib)
  else ()
    message (FATAL_ERROR "Bundled zlib did not create zlibstatic or zlib")
  endif ()
endif ()
set (ZLIB_INCLUDE_DIR "${zlib_SOURCE_DIR};${zlib_BINARY_DIR}")
set (ZLIB_INCLUDE_DIRS "${zlib_SOURCE_DIR}" "${zlib_BINARY_DIR}")
set (ZLIB_FOUND TRUE)

# libpng calls find_package(ZLIB); prefer our in-tree target over the module.
file (WRITE "${CMAKE_BINARY_DIR}/cmake-overrides/FindZLIB.cmake"
"if (TARGET ZLIB::ZLIB)
  set (ZLIB_FOUND TRUE)
  set (ZLIB_INCLUDE_DIR \"${zlib_SOURCE_DIR}\" \"${zlib_BINARY_DIR}\")
  set (ZLIB_INCLUDE_DIRS \"${zlib_SOURCE_DIR}\" \"${zlib_BINARY_DIR}\")
  set (ZLIB_LIBRARIES ZLIB::ZLIB)
  set (ZLIB_LIBRARY ZLIB::ZLIB)
  return ()
endif ()
")
list (PREPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}/cmake-overrides")

if (COMPILER_ONLY)
  set (BUILD_SHARED_LIBS "${_bennugd_saved_shared}")
  set (CMAKE_SKIP_INSTALL_RULES "${_bennugd_saved_skip_install}")
  return ()
endif ()

# --- libpng ---
set (PNG_SHARED OFF CACHE BOOL "" FORCE)
set (PNG_STATIC ON CACHE BOOL "" FORCE)
set (PNG_FRAMEWORK OFF CACHE BOOL "" FORCE)
set (PNG_TESTS OFF CACHE BOOL "" FORCE)
set (PNG_TOOLS OFF CACHE BOOL "" FORCE)
FetchContent_Declare (
  libpng
  URL "https://github.com/pnggroup/libpng/archive/refs/tags/v${BENNUGD_LIBPNG_VERSION}.tar.gz"
)
FetchContent_GetProperties (libpng)
if (NOT libpng_POPULATED)
  FetchContent_Populate (libpng)
  add_subdirectory (${libpng_SOURCE_DIR} ${libpng_BINARY_DIR} EXCLUDE_FROM_ALL)
endif ()

if (NOT TARGET PNG::PNG)
  if (TARGET png_static)
    add_library (PNG::PNG ALIAS png_static)
  elseif (TARGET png)
    add_library (PNG::PNG ALIAS png)
  else ()
    message (FATAL_ERROR "Bundled libpng did not create png_static or png")
  endif ()
endif ()

# --- SDL3 ---
# SDLActivity loads libSDL3.so + libmain.so. Other targets link SDL statically.
if (ANDROID)
  set (SDL_SHARED ON CACHE BOOL "" FORCE)
  set (SDL_STATIC OFF CACHE BOOL "" FORCE)
else ()
  set (SDL_SHARED OFF CACHE BOOL "" FORCE)
  set (SDL_STATIC ON CACHE BOOL "" FORCE)
endif ()
set (SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set (SDL_TESTS OFF CACHE BOOL "" FORCE)
set (SDL_INSTALL_DOCS OFF CACHE BOOL "" FORCE)
set (SDL_LIBUSB OFF CACHE BOOL "" FORCE)
set (SDL_HIDAPI_LIBUSB OFF CACHE BOOL "" FORCE)
if (EMSCRIPTEN)
  # Browser Gamepad API only. SDL's virtual joystick appears as a fake pad
  # at init and makes games assign P1 to a stick that is not a real device.
  set (SDL_VIRTUAL_JOYSTICK OFF CACHE BOOL "" FORCE)
endif ()
if (NINTENDO_SWITCH)
  # newlib has no iconv; static try_compile must not claim GNU libc extras.
  set (SDL_SYSTEM_ICONV OFF CACHE BOOL "" FORCE)
endif ()
if (PLATFORM_DREAMCAST OR DREAMCAST)
  set (SDL_SYSTEM_ICONV OFF CACHE BOOL "" FORCE)
  set (SDL_OPENGL OFF CACHE BOOL "" FORCE)
  set (SDL_OPENGLES OFF CACHE BOOL "" FORCE)
  set (SDL_RENDER_GPU OFF CACHE BOOL "" FORCE)
  set (SDL_HIDAPI OFF CACHE BOOL "" FORCE)
  set (SDL_VIRTUAL_JOYSTICK OFF CACHE BOOL "" FORCE)
endif ()
if (PLATFORM_PANDORA OR OPENPANDORA)
  # Ångström sysroot is glibc 2.9 + X11 + ALSA; no Wayland/GLES/Pulse.
  # X11 extras: only Xext/Xrandr/Xrender are in the sysroot (no Xi/Xcursor).
  set (SDL_SYSTEM_ICONV OFF CACHE BOOL "" FORCE)
  set (SDL_UNIX_CONSOLE_BUILD OFF CACHE BOOL "" FORCE)
  set (SDL_X11 ON CACHE BOOL "" FORCE)
  set (SDL_X11_XCURSOR OFF CACHE BOOL "" FORCE)
  set (SDL_X11_XINPUT OFF CACHE BOOL "" FORCE)
  set (SDL_X11_XFIXES OFF CACHE BOOL "" FORCE)
  set (SDL_X11_XSCRNSAVER OFF CACHE BOOL "" FORCE)
  set (SDL_X11_XTEST OFF CACHE BOOL "" FORCE)
  set (SDL_X11_XRANDR ON CACHE BOOL "" FORCE)
  set (SDL_WAYLAND OFF CACHE BOOL "" FORCE)
  set (SDL_KMSDRM OFF CACHE BOOL "" FORCE)
  set (SDL_OPENGL OFF CACHE BOOL "" FORCE)
  set (SDL_OPENGLES OFF CACHE BOOL "" FORCE)
  set (SDL_VULKAN OFF CACHE BOOL "" FORCE)
  set (SDL_RENDER_GPU OFF CACHE BOOL "" FORCE)
  set (SDL_HIDAPI OFF CACHE BOOL "" FORCE)
  set (SDL_IBUS OFF CACHE BOOL "" FORCE)
  set (SDL_DBUS OFF CACHE BOOL "" FORCE)
  set (SDL_LIBUDEV OFF CACHE BOOL "" FORCE)
  set (SDL_PIPEWIRE OFF CACHE BOOL "" FORCE)
  set (SDL_PULSEAUDIO OFF CACHE BOOL "" FORCE)
  set (SDL_ALSA ON CACHE BOOL "" FORCE)
  set (SDL_JACK OFF CACHE BOOL "" FORCE)
  set (SDL_SNDIO OFF CACHE BOOL "" FORCE)
endif ()
if (PLATFORM_PSP OR PSP)
  # pspdev already ships SDL3/SDL3_mixer built for Allegrex.
  find_package (SDL3 REQUIRED CONFIG)
  if (NOT NO_SOUND)
    find_package (SDL3_mixer REQUIRED CONFIG)
  endif ()
else ()
FetchContent_Declare (
  sdl3
  GIT_REPOSITORY "${BENNUGD_SDL3_GIT_REPOSITORY}"
  GIT_TAG "${BENNUGD_SDL3_REF}"
  GIT_SHALLOW TRUE
)
FetchContent_GetProperties (sdl3)
if (NOT sdl3_POPULATED)
  FetchContent_Populate (sdl3)
  add_subdirectory (${sdl3_SOURCE_DIR} ${sdl3_BINARY_DIR} EXCLUDE_FROM_ALL)
endif ()

if (PLATFORM_DREAMCAST OR DREAMCAST)
  # GPF SDL tracks keys via key_states[]; KallistiOS 2.x uses matrix[].
  set (_kbd "${sdl3_SOURCE_DIR}/src/video/dreamcast/SDL_dreamcastkeyboard.c")
  if (EXISTS "${_kbd}")
    file (READ "${_kbd}" _kbd_txt)
    if (_kbd_txt MATCHES "key_states\\[i\\]\\.was_down")
      string (REPLACE
        "bool was_down = state->key_states[i].was_down;\n        bool is_down = state->key_states[i].is_down;"
        "bool is_down = (state->matrix[i] == KEY_STATE_PRESSED);"
        _kbd_txt "${_kbd_txt}")
      file (WRITE "${_kbd}" "${_kbd_txt}")
    endif ()
  endif ()
  # CreateWindowFramebuffer only allocates sdl_dc_buf[] when DOUBLE_BUFFER is
  # set; UpdateWindowFramebuffer still defaults that hint to true and sq_cpy
  # from a NULL backbuffer (first frame visible, then VRAM wiped).
  set (_fb "${sdl3_SOURCE_DIR}/src/video/dreamcast/SDL_dreamcastframebuffer.c")
  if (EXISTS "${_fb}")
    file (READ "${_fb}" _fb_txt)
    if (_fb_txt MATCHES "if \\(double_buffer\\) \\{\n            sq_cpy\\(vram_l, sdl_dc_buf")
      string (REPLACE
        "if (double_buffer) {\n            sq_cpy(vram_l, sdl_dc_buf[sdl_dc_buf_index], h * pitch);"
        "if (double_buffer && sdl_dc_buf[sdl_dc_buf_index]) {\n            sq_cpy(vram_l, sdl_dc_buf[sdl_dc_buf_index], h * pitch);"
        _fb_txt "${_fb_txt}")
      file (WRITE "${_fb}" "${_fb_txt}")
    endif ()
  endif ()
endif ()

if (PLATFORM_PANDORA OR OPENPANDORA)
  # Ångström ALSA 1.0.20 has no snd_pcm_chmap_* (added in 1.0.24). Keep
  # stereo/default layout and do not require those symbols at dlopen.
  set (_alsa "${sdl3_SOURCE_DIR}/src/audio/alsa/SDL_alsa_audio.c")
  if (EXISTS "${_alsa}")
    file (READ "${_alsa}" _alsa_txt)
    if (NOT _alsa_txt MATCHES "BENNUGD_PANDORA_ALSA_CHMAP")
      string (REPLACE
        "#include \"SDL_alsa_audio.h\"\n#include \"../../core/linux/SDL_udev.h\""
        "#include \"SDL_alsa_audio.h\"\n#include \"../../core/linux/SDL_udev.h\"\n\n#ifndef SND_CHMAP_TYPE_VAR\n/* BENNUGD_PANDORA_ALSA_CHMAP */\ntypedef enum { SND_CHMAP_TYPE_NONE = 0, SND_CHMAP_TYPE_FIXED, SND_CHMAP_TYPE_VAR, SND_CHMAP_TYPE_PAIRED } snd_pcm_chmap_type_t;\nenum snd_pcm_chmap_position { SND_CHMAP_UNKNOWN = 0, SND_CHMAP_NA, SND_CHMAP_MONO, SND_CHMAP_FL, SND_CHMAP_FR, SND_CHMAP_FC, SND_CHMAP_LFE, SND_CHMAP_RL, SND_CHMAP_RR, SND_CHMAP_RC, SND_CHMAP_SL, SND_CHMAP_SR };\ntypedef struct { unsigned int channels; unsigned int pos[128]; } snd_pcm_chmap_t;\ntypedef struct { snd_pcm_chmap_type_t type; snd_pcm_chmap_t map; } snd_pcm_chmap_query_t;\n#endif\n"
        _alsa_txt "${_alsa_txt}")
      string (REPLACE
        "    SDL_ALSA_SYM(snd_pcm_query_chmaps);\n    SDL_ALSA_SYM(snd_pcm_free_chmaps);\n    SDL_ALSA_SYM(snd_pcm_set_chmap);\n    SDL_ALSA_SYM(snd_pcm_chmap_print);"
        "#ifdef SDL_AUDIO_DRIVER_ALSA_DYNAMIC\n#define SDL_ALSA_SYM_OPTIONAL(x) load_alsa_sym(#x, (void **)(char *)&ALSA_##x)\n#else\n#define SDL_ALSA_SYM_OPTIONAL(x) ALSA_##x = x\n#endif\n    SDL_ALSA_SYM_OPTIONAL(snd_pcm_query_chmaps);\n    SDL_ALSA_SYM_OPTIONAL(snd_pcm_free_chmaps);\n    SDL_ALSA_SYM_OPTIONAL(snd_pcm_set_chmap);\n    SDL_ALSA_SYM_OPTIONAL(snd_pcm_chmap_print);\n    SDL_ClearError();"
        _alsa_txt "${_alsa_txt}")
      string (REPLACE
        "    ctx->chmap_queries = ALSA_snd_pcm_query_chmaps(ctx->device->hidden->pcm);"
        "    if (!ALSA_snd_pcm_query_chmaps) {\n        ctx->chmap_queries = NULL;\n        LOGDEBUG(\"channel map API missing, swizzling off\");\n        return CHMAP_INSTALLED;\n    }\n    ctx->chmap_queries = ALSA_snd_pcm_query_chmaps(ctx->device->hidden->pcm);"
        _alsa_txt "${_alsa_txt}")
      string (REPLACE
        "        ALSA_snd_pcm_free_chmaps(ctx->chmap_queries);"
        "        if (ALSA_snd_pcm_free_chmaps) ALSA_snd_pcm_free_chmaps(ctx->chmap_queries);"
        _alsa_txt "${_alsa_txt}")
      string (REPLACE
        "    ALSA_snd_pcm_free_chmaps(cfg_ctx.chmap_queries);"
        "    if (ALSA_snd_pcm_free_chmaps) ALSA_snd_pcm_free_chmaps(cfg_ctx.chmap_queries);"
        _alsa_txt "${_alsa_txt}")
      file (WRITE "${_alsa}" "${_alsa_txt}")
    endif ()
  endif ()
  set (_evdev "${sdl3_SOURCE_DIR}/src/core/linux/SDL_evdev_capabilities.h")
  if (EXISTS "${_evdev}")
    file (READ "${_evdev}" _evdev_txt)
    if (NOT _evdev_txt MATCHES "INPUT_PROP_BUTTONPAD")
      string (REPLACE
        "#ifndef INPUT_PROP_SEMI_MT\n#define INPUT_PROP_SEMI_MT          0x03\n#endif"
        "#ifndef INPUT_PROP_BUTTONPAD\n#define INPUT_PROP_BUTTONPAD        0x02\n#endif\n#ifndef INPUT_PROP_SEMI_MT\n#define INPUT_PROP_SEMI_MT          0x03\n#endif"
        _evdev_txt "${_evdev_txt}")
      file (WRITE "${_evdev}" "${_evdev_txt}")
    endif ()
  endif ()
  set (_joy "${sdl3_SOURCE_DIR}/src/joystick/linux/SDL_sysjoystick.c")
  if (EXISTS "${_joy}")
    file (READ "${_joy}" _joy_txt)
    if (NOT _joy_txt MATCHES "ifndef EVIOCGPROP")
      string (REPLACE
        "#ifndef MSC_TIMESTAMP\n#define MSC_TIMESTAMP 0x05\n#endif"
        "#ifndef MSC_TIMESTAMP\n#define MSC_TIMESTAMP 0x05\n#endif\n#ifndef EVIOCGPROP\n#define EVIOCGPROP(len) _IOC(_IOC_READ, 'E', 0x09, len)\n#endif"
        _joy_txt "${_joy_txt}")
      file (WRITE "${_joy}" "${_joy_txt}")
    endif ()
  endif ()
endif ()

if (EXISTS "${sdl3_BINARY_DIR}/SDL3Config.cmake")
  set (SDL3_DIR "${sdl3_BINARY_DIR}" CACHE PATH "" FORCE)
elseif (EXISTS "${sdl3_BINARY_DIR}/cmake/SDL3Config.cmake")
  set (SDL3_DIR "${sdl3_BINARY_DIR}/cmake" CACHE PATH "" FORCE)
endif ()
list (PREPEND CMAKE_PREFIX_PATH "${sdl3_BINARY_DIR}")

# --- SDL3_mixer ---
if (NOT NO_SOUND)
  set (SDLMIXER_DEPS_SHARED OFF CACHE BOOL "" FORCE)
  set (SDLMIXER_VENDORED OFF CACHE BOOL "" FORCE)
  set (SDLMIXER_EXAMPLES OFF CACHE BOOL "" FORCE)
  set (SDLMIXER_TESTS OFF CACHE BOOL "" FORCE)
  set (SDLMIXER_STRICT OFF CACHE BOOL "" FORCE)
  set (SDLMIXER_GME OFF CACHE BOOL "" FORCE)
  set (SDLMIXER_MOD OFF CACHE BOOL "" FORCE)
  set (SDLMIXER_MIDI OFF CACHE BOOL "" FORCE)
  set (SDLMIXER_WAVPACK OFF CACHE BOOL "" FORCE)
  set (SDLMIXER_FLAC OFF CACHE BOOL "" FORCE)
  set (SDLMIXER_OPUS OFF CACHE BOOL "" FORCE)
  set (SDLMIXER_MP3 ON CACHE BOOL "" FORCE)
  set (SDLMIXER_MP3_DRMP3 ON CACHE BOOL "" FORCE)
  set (SDLMIXER_MP3_MPG123 OFF CACHE BOOL "" FORCE)
  set (SDLMIXER_VORBIS_STB ON CACHE BOOL "" FORCE)
  set (SDLMIXER_VORBIS_VORBISFILE OFF CACHE BOOL "" FORCE)
  set (SDLMIXER_VORBIS_TREMOR OFF CACHE BOOL "" FORCE)
  FetchContent_Declare (
    sdl3_mixer
    GIT_REPOSITORY "https://github.com/libsdl-org/SDL_mixer.git"
    GIT_TAG "${BENNUGD_SDL3_MIXER_REF}"
    GIT_SHALLOW TRUE
  )
  FetchContent_GetProperties (sdl3_mixer)
  if (NOT sdl3_mixer_POPULATED)
    FetchContent_Populate (sdl3_mixer)
    if (MSVC)
      set (_stb "${sdl3_mixer_SOURCE_DIR}/src/decoder_stb_vorbis.c")
      if (EXISTS "${_stb}")
        file (READ "${_stb}" _stb_txt)
        string (REPLACE
          "#define STB_FORCEINLINE SDL_FORCE_INLINE"
          "#define STB_FORCEINLINE static SDL_FORCE_INLINE"
          _stb_txt "${_stb_txt}")
        file (WRITE "${_stb}" "${_stb_txt}")
      endif ()
    endif ()
    if (PLATFORM_DREAMCAST OR DREAMCAST)
      set (_stb "${sdl3_mixer_SOURCE_DIR}/src/decoder_stb_vorbis.c")
      if (EXISTS "${_stb}")
        file (READ "${_stb}" _stb_txt)
        string (REPLACE
          "typedef Uint8 uint8;\ntypedef Sint8 int8;\ntypedef Uint16 uint16;\ntypedef Sint16 int16;\ntypedef Uint32 uint32;\ntypedef Sint32 int32;\n"
          "/* KallistiOS already typedefs int8/uint8/... in arch/types.h */\n"
          _stb_txt "${_stb_txt}")
        file (WRITE "${_stb}" "${_stb_txt}")
      endif ()
    endif ()
    add_subdirectory (${sdl3_mixer_SOURCE_DIR} ${sdl3_mixer_BINARY_DIR} EXCLUDE_FROM_ALL)
  endif ()
endif ()
endif ()

set (BUILD_SHARED_LIBS "${_bennugd_saved_shared}")
set (CMAKE_SKIP_INSTALL_RULES "${_bennugd_saved_skip_install}")
