# GPF/SDL (dreamcastSDL3) does include(kallistios). Current KallistiOS ships
# the same helpers as dreamcast.cmake and already includes them from the
# toolchain file. This stub avoids a configure failure when the module name
# has not been renamed yet.

if (NOT COMMAND kos_add_romdisk AND DEFINED ENV{KOS_BASE})
  if (EXISTS "$ENV{KOS_BASE}/utils/cmake/kallistios.cmake")
    include ("$ENV{KOS_BASE}/utils/cmake/kallistios.cmake")
  elseif (EXISTS "$ENV{KOS_BASE}/utils/cmake/dreamcast.cmake")
    include ("$ENV{KOS_BASE}/utils/cmake/dreamcast.cmake")
  endif ()
endif ()
