add_library (
  des
  ${MODULES}
  cbc_cksm.c
  pcbc_enc.c
  qud_cksm.c
  cbc3_enc.c
  cfb64ede.c
  cfb64enc.c
  cfb_enc.c
  ecb3_enc.c
  enc_read.c
  enc_writ.c
  ofb64ede.c
  ofb64enc.c
  ofb_enc.c
  rand_key.c
  rpc_enc.c
  str2key.c
  supp.c
  xcbc_enc.c
  cbc_enc.c
  ecb_enc.c
  fcrypt.c
  set_key.c
  des_enc.c
  fcrypt_b.c
)

if (NOT EMSCRIPTEN AND NOT CMAKE_SYSTEM_NAME MATCHES "WASI" AND NOT NINTENDO_SWITCH AND NOT PLATFORM_DREAMCAST AND NOT DREAMCAST AND NOT PLATFORM_PSP AND NOT PSP AND NOT PLATFORM_VITA AND NOT VITA AND NOT PLATFORM_PS2 AND NOT PS2 AND NOT PLATFORM_PS3 AND NOT PS3 AND NOT PLATFORM_PS4 AND NOT PS4 AND NOT NINTENDO_WII AND NOT PLATFORM_WII AND NOT CMAKE_SYSTEM_NAME STREQUAL "tvOS" AND NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
  target_sources (des PRIVATE
    read_pwd.c
    read2pwd.c
  )
endif ()
