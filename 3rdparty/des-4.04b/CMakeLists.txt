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

if (NOT EMSCRIPTEN)
  target_sources (des PRIVATE
    read_pwd.c
    read2pwd.c
  )
endif ()
