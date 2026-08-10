# BennuGD64

A fork of [BennuGD](https://www.bennugd.org/) adapted to run on modern 64-bit architectures.

## Dependencies

- CMake
- SDL3
- SDL3_mixer (unless `NO_SOUND` is enabled)
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

### Static dependencies (CI / portable builds)

```shell
./scripts/ci/build-static-deps.sh
./scripts/ci/build-bennugd.sh
./scripts/ci/package.sh
```

Useful CMake options:

| Option | Default | Description |
|--------|---------|-------------|
| `BENNUGD_STATIC_DEPS` | OFF | Prefer static zlib/libpng/SDL3/SDL3_mixer |
| `USE_LIBDES` | OFF | Use bundled DES instead of OpenSSL |
| `STATIC_MODULES` | ON | Link modules into `bgdi` |
| `NO_SOUND` | OFF | Build without SDL3_mixer |

## CI/CD

GitHub Actions (`.github/workflows/ci.yml`) builds `bgdc` and `bgdi` for:

- Linux x86_64
- Windows x86_64 (MinGW-w64 UCRT64 via MSYS2)
- macOS arm64

On tags matching `v*` (for example `v1.0.0`), the workflow publishes a GitHub Release with archives that embed zlib, libpng, SDL3, SDL3_mixer and the bundled DES library statically. OS graphics/audio libraries may still be required at runtime.
