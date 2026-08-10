# BennuGD64

A fork of [BennuGD](https://www.bennugd.org/) adapted to run on modern 64-bit architectures.

## Dependencies

- CMake
- SDL2
- SDL2_mixer (unless `NO_SOUND` is enabled)
- zlib
- OpenSSL (or set `USE_LIBDES` to use the bundled DES library)
- libpng

## Build

Generate both the compiler `bgdc` and the interpreter `bgdi`:

```shell
cmake -S . -B build
cmake --build build
```

Binaries:

| Tool | Path |
|------|------|
| bgdc | build/core/bgdc/src/bgdc |
| bgdi | build/core/bgdi/src/bgdi |
