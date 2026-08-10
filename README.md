# BennuGD (CMake)

## Dependencies

- CMake
- SDL
- zlib
- OpenSSL (or set `USE_LIBDES` to use the bundled DES library)
- libX11
- SDL_mixer (unless `NO_SOUND` is enabled)
- libpng

## Build

Generate both the compiler (`bgdc`) and the interpreter (`bgdi`):

```shell
cmake -S . -B build
cmake --build build
```

Binaries:

| Tool | Path |
|------|------|
| `bgdc` | `build/core/bgdc/src/bgdc` |
| `bgdi` | `build/core/bgdi/src/bgdi` |

### Interpreter only

To build only `bgdi`, set in the top-level `CMakeLists.txt`:

```cmake
set (INTERPRETER_ONLY true)
```

Then reconfigure and build:

```shell
cmake -S . -B build
cmake --build build
```
