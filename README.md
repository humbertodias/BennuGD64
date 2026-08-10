# BennuGD

[BennuGD](https://www.bennugd.org/) is a high-level, open-source game development suite that focuses on modularity and portability, making it an excellent choice for cross-platform game development

## Dependencies

- CMake
- SDL
- zlib
- OpenSSL (or set `USE_LIBDES` to use the bundled DES library)
- libX11
- SDL_mixer (unless `NO_SOUND` is enabled)
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
