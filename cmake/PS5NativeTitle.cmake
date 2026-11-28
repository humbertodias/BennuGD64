# Reuse the interpreter's sources and libraries with the native title CRT.
set (PS5_NATIVE_TOOLS "/opt/ps5-native" CACHE PATH "PS5 native application tools")
set (_ps5_native_sdk_lib "${PS5_PAYLOAD_SDK}/target/lib")
set (_ps5_native_libc "${CMAKE_CURRENT_BINARY_DIR}/libc-native.a")
set (_ps5_native_layout "${PS5_NATIVE_TOOLS}/ps5-pie.ld")
set (_ps5_native_symbols "${PS5_NATIVE_TOOLS}/app-symbols.map")
set (_ps5_native_crt "${PS5_NATIVE_TOOLS}/app_crt.cpp")

foreach (_ps5_native_file IN ITEMS "${_ps5_native_crt}" "${_ps5_native_layout}"
    "${_ps5_native_symbols}" "${PS5_NATIVE_TOOLS}/ps5-native-tool")
  if (NOT EXISTS "${_ps5_native_file}")
    message (FATAL_ERROR "Missing PS5 native title toolchain file: ${_ps5_native_file}")
  endif ()
endforeach ()

# In a native title, syscalls must go through the system kernel module.
add_custom_command (
  OUTPUT "${_ps5_native_libc}"
  COMMAND "${CMAKE_COMMAND}" -E copy "${_ps5_native_sdk_lib}/libc.a" "${_ps5_native_libc}"
  COMMAND "${CMAKE_AR}" d "${_ps5_native_libc}" syscalls.o mman.o
  DEPENDS "${_ps5_native_sdk_lib}/libc.a"
  VERBATIM
)
add_custom_target (bgdi_ps5_native_libc DEPENDS "${_ps5_native_libc}")

get_target_property (_ps5_native_sources bgdi SOURCES)
get_target_property (_ps5_native_libraries bgdi LINK_LIBRARIES)
list (REMOVE_ITEM _ps5_native_libraries pthread m)
add_library (bgdi_ps5_native_crt OBJECT "${_ps5_native_crt}")
# The CRT uses only SDK headers. The legacy DES include path has a
# lowercase "version" file that can shadow libc++'s <version> on macOS.
set_property (TARGET bgdi_ps5_native_crt PROPERTY INCLUDE_DIRECTORIES "")
target_compile_features (bgdi_ps5_native_crt PRIVATE cxx_std_20)
add_executable (bgdi_ps5_native_pie EXCLUDE_FROM_ALL
  ${_ps5_native_sources}
  ps5_native_runtime.c
  ps5_native_heap.c
  $<TARGET_OBJECTS:bgdi_ps5_native_crt>
)
add_dependencies (bgdi_ps5_native_pie bennugd_git_header bgdi_ps5_native_libc)
target_compile_definitions (bgdi_ps5_native_pie PRIVATE BENNUGD_PS5_NATIVE_TITLE=1)
target_link_libraries (bgdi_ps5_native_pie PRIVATE
  ${_ps5_native_libraries}
  kernel SceLibcInternal
  "${_ps5_native_libc}"
)
target_link_options (bgdi_ps5_native_pie PRIVATE
  -nostartfiles -nodefaultlibs
  "LINKER:-T,${_ps5_native_layout}"
  "LINKER:--version-script,${_ps5_native_symbols}"
  "LINKER:--wrap=stat,--wrap=exit,--wrap=_exit,--wrap=_Exit"
  "LINKER:--wrap=malloc,--wrap=calloc,--wrap=realloc,--wrap=free,--wrap=posix_memalign,--wrap=aligned_alloc,--wrap=strdup"
  "LINKER:-e,_start,--eh-frame-hdr,--no-dynamic-linker,--as-needed,--gc-sections"
)
set_target_properties (bgdi_ps5_native_pie PROPERTIES
  LINKER_LANGUAGE C
  SUFFIX ".elf"
  LINK_DEPENDS "${_ps5_native_layout};${_ps5_native_symbols}"
)

set (_ps5_native_elf "${CMAKE_CURRENT_BINARY_DIR}/bgdi-native.elf")
set (_ps5_native_eboot "${CMAKE_CURRENT_BINARY_DIR}/eboot.bin")
add_custom_command (
  OUTPUT "${_ps5_native_eboot}"
  COMMAND "${PS5_NATIVE_TOOLS}/ps5-native-tool" link
    --in "$<TARGET_FILE:bgdi_ps5_native_pie>" --out "${_ps5_native_elf}"
    --stub-dir "${_ps5_native_sdk_lib}" --file-name eboot.elf
    --module-sdk 0x02000009 --companion-sdk 0x08050001 --heap-size 0xffffffffffffffff
  COMMAND "${PS5_NATIVE_TOOLS}/ps5-native-tool" self --sign
    --in "${_ps5_native_elf}" --out "${_ps5_native_eboot}" --magic 0x1D3D154F
  DEPENDS bgdi_ps5_native_pie "${PS5_NATIVE_TOOLS}/ps5-native-tool"
  BYPRODUCTS "${_ps5_native_elf}"
  VERBATIM
)
add_custom_target (bgdi_ps5_native DEPENDS "${_ps5_native_eboot}")
