# Relocatable layout used by `cmake --install` (and CI artifacts):
#   <prefix>/bgdc  <prefix>/bgdi
#   <prefix>/libbgdrtm.*  <prefix>/libdes.*   (shared builds)
#   <prefix>/modules/*                        (shared builds)
#   <prefix>/README.md  <prefix>/BUILD_INFO.txt

if (DEFINED BENNUGD_RESOLVED_VERSION AND NOT BENNUGD_RESOLVED_VERSION STREQUAL "")
  set (BENNUGD_PACKAGE_VERSION "${BENNUGD_RESOLVED_VERSION}")
elseif (NOT DEFINED BENNUGD_PACKAGE_VERSION OR BENNUGD_PACKAGE_VERSION STREQUAL "")
  set (BENNUGD_PACKAGE_VERSION "dev")
endif ()

if (EMSCRIPTEN)
  set (BENNUGD_PACKAGE_OS "web")
elseif (ANDROID)
  set (BENNUGD_PACKAGE_OS "android")
elseif (NINTENDO_SWITCH)
  set (BENNUGD_PACKAGE_OS "switch")
elseif (PLATFORM_DREAMCAST OR DREAMCAST)
  set (BENNUGD_PACKAGE_OS "dreamcast")
elseif (PLATFORM_PSP OR PSP)
  set (BENNUGD_PACKAGE_OS "psp")
elseif (PLATFORM_PANDORA OR OPENPANDORA)
  set (BENNUGD_PACKAGE_OS "pandora")
elseif (CMAKE_SYSTEM_NAME MATCHES "WASI")
  set (BENNUGD_PACKAGE_OS "wasi")
elseif (WIN32)
  set (BENNUGD_PACKAGE_OS "windows")
elseif (APPLE)
  set (BENNUGD_PACKAGE_OS "macos")
else ()
  set (BENNUGD_PACKAGE_OS "linux")
endif ()

if (EMSCRIPTEN OR CMAKE_SYSTEM_NAME MATCHES "WASI")
  set (BENNUGD_PACKAGE_ARCH "wasm32")
elseif (ANDROID)
  if (ANDROID_ABI STREQUAL "arm64-v8a")
    set (BENNUGD_PACKAGE_ARCH "arm64")
  else ()
    set (BENNUGD_PACKAGE_ARCH "${ANDROID_ABI}")
  endif ()
elseif (NINTENDO_SWITCH)
  set (BENNUGD_PACKAGE_ARCH "aarch64")
elseif (PLATFORM_DREAMCAST OR DREAMCAST)
  set (BENNUGD_PACKAGE_ARCH "sh4")
elseif (PLATFORM_PSP OR PSP)
  set (BENNUGD_PACKAGE_ARCH "mips")
elseif (PLATFORM_PANDORA OR OPENPANDORA)
  set (BENNUGD_PACKAGE_ARCH "arm")
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64|arm64")
  set (BENNUGD_PACKAGE_ARCH "arm64")
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|amd64")
  set (BENNUGD_PACKAGE_ARCH "x86_64")
else ()
  set (BENNUGD_PACKAGE_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
endif ()

if (CMAKE_SYSTEM_NAME MATCHES "WASI")
  set (BENNUGD_PACKAGE_LINKAGE "static")
  set (BENNUGD_PACKAGE_MODULES_NOTE "compiler-only; no interpreter")
  set (BENNUGD_PACKAGE_DEPS "zlib, bundled DES")
  set (BENNUGD_PACKAGE_NOTE "Run with Wasmtime: wasmtime --dir=. ./bgdc.wasm -- -o out.dcb in.prg")
elseif (ANDROID)
  set (BENNUGD_PACKAGE_LINKAGE "static")
  set (BENNUGD_PACKAGE_MODULES_NOTE "linked into libmain.so")
  set (BENNUGD_PACKAGE_DEPS "zlib, libpng, SDL3, SDL3_mixer (stb_vorbis + dr_mp3), bundled DES")
  set (BENNUGD_PACKAGE_NOTE "Install the APK. Put a main.dcb in APK assets/ or the app files dir.")
elseif (NINTENDO_SWITCH)
  set (BENNUGD_PACKAGE_LINKAGE "static")
  set (BENNUGD_PACKAGE_MODULES_NOTE "linked into bgdi.elf / bennugd64.nro")
  set (BENNUGD_PACKAGE_DEPS "zlib, libpng, SDL3 (devkitPro switch-sdl-3.4), SDL3_mixer (stb_vorbis + dr_mp3), bundled DES, libnx")
  set (BENNUGD_PACKAGE_NOTE "Copy bennugd64.nro to the Switch (sdmc:/switch/) and launch it, or send with nxlink.")
elseif (PLATFORM_DREAMCAST OR DREAMCAST)
  set (BENNUGD_PACKAGE_LINKAGE "static")
  set (BENNUGD_PACKAGE_MODULES_NOTE "linked into bgdi.elf / bennugd64.cdi")
  set (BENNUGD_PACKAGE_DEPS "zlib, libpng, SDL3 (GPF dreamcastSDL3), SDL3_mixer (stb_vorbis + dr_mp3), bundled DES, KallistiOS")
  set (BENNUGD_PACKAGE_NOTE "Burn or emulate bennugd64.cdi, or send bgdi.elf with dc-tool. The ISO ships hello.dcb as main.dcb.")
elseif (PLATFORM_PSP OR PSP)
  set (BENNUGD_PACKAGE_LINKAGE "static")
  set (BENNUGD_PACKAGE_MODULES_NOTE "linked into bgdi.elf / EBOOT.PBP")
  set (BENNUGD_PACKAGE_DEPS "zlib, libpng, SDL3 (pspdev 3.4.10), SDL3_mixer (pspdev 3.2.4), bundled DES, pspsdk")
  set (BENNUGD_PACKAGE_NOTE "Copy the folder to ms0:/PSP/GAME/bennugd64/ (EBOOT.PBP + main.dcb).")
elseif (PLATFORM_PANDORA OR OPENPANDORA)
  set (BENNUGD_PACKAGE_LINKAGE "static")
  set (BENNUGD_PACKAGE_MODULES_NOTE "linked into bgdi / bennugd64.pnd")
  set (BENNUGD_PACKAGE_DEPS "zlib, libpng, SDL3 (X11 software), SDL3_mixer (stb_vorbis + dr_mp3), bundled DES, Ångström glibc 2.9")
  set (BENNUGD_PACKAGE_NOTE "Copy bennugd64.pnd to the Pandora SD card, or run ./bgdi next to main.dcb.")
elseif (EMSCRIPTEN)
  set (BENNUGD_PACKAGE_LINKAGE "static")
  set (BENNUGD_PACKAGE_MODULES_NOTE "linked into bgdi")
  set (BENNUGD_PACKAGE_DEPS "zlib, libpng, SDL3, SDL3_mixer (stb_vorbis + dr_mp3), bundled DES")
  set (BENNUGD_PACKAGE_NOTE "Serve the folder over HTTP and open index.html.")
elseif (STATIC_MODULES)
  set (BENNUGD_PACKAGE_LINKAGE "static")
  set (BENNUGD_PACKAGE_MODULES_NOTE "linked into bgdi")
  set (BENNUGD_PACKAGE_DEPS "zlib, libpng, SDL3, SDL3_mixer (stb_vorbis + dr_mp3), bundled DES")
  set (BENNUGD_PACKAGE_NOTE "OS graphics/audio system libraries may still be required at runtime (X11/Wayland/Cocoa/DirectX).")
else ()
  set (BENNUGD_PACKAGE_LINKAGE "shared")
  set (BENNUGD_PACKAGE_MODULES_NOTE "loaded from modules/")
  set (BENNUGD_PACKAGE_DEPS "zlib, libpng, SDL3, SDL3_mixer (stb_vorbis + dr_mp3), bundled DES")
  set (BENNUGD_PACKAGE_NOTE "OS graphics/audio system libraries may still be required at runtime (X11/Wayland/Cocoa/DirectX).")
endif ()

set (BENNUGD_PACKAGE_NAME
  "bennugd64-${BENNUGD_PACKAGE_VERSION}-${BENNUGD_PACKAGE_OS}-${BENNUGD_PACKAGE_ARCH}-${BENNUGD_PACKAGE_LINKAGE}"
)

configure_file (
  ${CMAKE_SOURCE_DIR}/cmake/BUILD_INFO.txt.in
  ${CMAKE_BINARY_DIR}/BUILD_INFO.txt
  @ONLY
)

set (_bennugd_bins)
if (TARGET bgdi)
  list (APPEND _bennugd_bins bgdi)
endif ()
if (TARGET bgdc)
  list (APPEND _bennugd_bins bgdc)
endif ()
if (_bennugd_bins)
  if (ANDROID)
    install (TARGETS ${_bennugd_bins} LIBRARY DESTINATION .)
  else ()
    install (TARGETS ${_bennugd_bins} RUNTIME DESTINATION .)
  endif ()
endif ()

if (NOT STATIC_MODULES)
  install (TARGETS bgdrtm
    RUNTIME DESTINATION .
    LIBRARY DESTINATION .
  )
  if (TARGET des)
    install (TARGETS des
      RUNTIME DESTINATION .
      LIBRARY DESTINATION .
    )
  endif ()

  set (_bennugd_modules
    grbase video blit render draw font text sdlhandler wm key mouse joy scroll bgload
    mod_scroll mod_mouse mod_map mod_video mod_screen mod_blendop mod_text mod_draw
    mod_grproc mod_effects mod_key mod_cd mod_dir mod_file mod_crypt mod_joy
    mod_math mod_mathi mod_mem mod_proc mod_rand mod_regex mod_say mod_sort
    mod_sound mod_string mod_sys mod_time mod_timers mod_path mod_wm mod_debug
    mod_flic mod_m7
  )
  if (NO_SOUND)
    list (REMOVE_ITEM _bennugd_modules mod_sound)
  endif ()
  install (TARGETS ${_bennugd_modules}
    RUNTIME DESTINATION modules
    LIBRARY DESTINATION modules
  )
endif ()

install (FILES
  ${CMAKE_SOURCE_DIR}/README.md
  ${CMAKE_BINARY_DIR}/BUILD_INFO.txt
  DESTINATION .
)

if (MINGW)
  install (CODE [[
    set (_dest "${CMAKE_INSTALL_PREFIX}")
    execute_process (
      COMMAND x86_64-w64-mingw32-gcc -print-file-name=libgcc_s_seh-1.dll
      OUTPUT_VARIABLE _gcc_dll
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
    )
    foreach (_dll libgcc_s_seh-1.dll libwinpthread-1.dll libstdc++-6.dll)
      if (NOT EXISTS "${_dest}/${_dll}")
        set (_src "")
        if (_dll STREQUAL "libgcc_s_seh-1.dll" AND EXISTS "${_gcc_dll}" AND NOT _gcc_dll STREQUAL "libgcc_s_seh-1.dll")
          set (_src "${_gcc_dll}")
        elseif (EXISTS "/usr/x86_64-w64-mingw32/lib/${_dll}")
          set (_src "/usr/x86_64-w64-mingw32/lib/${_dll}")
        elseif (EXISTS "/usr/x86_64-w64-mingw32/bin/${_dll}")
          set (_src "/usr/x86_64-w64-mingw32/bin/${_dll}")
        endif ()
        if (NOT _src STREQUAL "")
          file (COPY "${_src}" DESTINATION "${_dest}")
        endif ()
      endif ()
    endforeach ()
  ]])
endif ()
