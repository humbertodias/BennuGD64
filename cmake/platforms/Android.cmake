# Android NDK: static modules, interpreter only (bgdi as libmain.so).
# minSdk / ANDROID_PLATFORM android-28 (bionic glob(3)). SDLActivity loads
# libSDL3.so + libmain.so.

set (STATIC_MODULES ON CACHE BOOL "Android loads one JNI library" FORCE)
set (INTERPRETER_ONLY ON CACHE BOOL "APK ships the interpreter; compile .prg on a host" FORCE)

# SDLActivity loads libSDL3.so + libmain.so.
set (SDL_SHARED ON CACHE BOOL "" FORCE)
set (SDL_STATIC OFF CACHE BOOL "" FORCE)

if (NOT USE_LIBDES)
  message (WARNING "Android builds should use -DUSE_LIBDES=ON (OpenSSL is not fetched for the NDK)")
endif ()
