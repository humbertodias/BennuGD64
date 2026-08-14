# Building BennuGD64

## Dependencies

- CMake
- SDL3
- SDL3_mixer (unless `NO_SOUND` is enabled)
- zlib
- OpenSSL (or set `USE_LIBDES` to use the bundled DES library)
- libpng

## Build

```shell
cmake -S . -B build
cmake --build build
```

Binaries:

| Tool | Path |
|------|------|
| bgdc | `build/core/bgdc/src/bgdc` |
| bgdi | `build/core/bgdi/src/bgdi` |

### Static dependencies (CI / portable builds)

```shell
./scripts/ci/build-static-deps.sh
STATIC_MODULES=ON LINKAGE=static ./scripts/ci/build-bennugd.sh
LINKAGE=static ./scripts/ci/package.sh
STATIC_MODULES=OFF LINKAGE=shared BUILD_DIR=build-ci-shared ./scripts/ci/build-bennugd.sh
LINKAGE=shared BUILD_DIR=build-ci-shared ./scripts/ci/package.sh
```

### CMake options

| Option | Default | Description |
|--------|---------|-------------|
| `BENNUGD_STATIC_DEPS` | OFF | Prefer static zlib/libpng/SDL3/SDL3_mixer |
| `USE_LIBDES` | OFF | Use bundled DES instead of OpenSSL |
| `STATIC_MODULES` | ON | Link modules into `bgdi` (`OFF` builds shared `.so`/`.dll`) |
| `NO_SOUND` | OFF | Build without SDL3_mixer |

Release archives are named `bennugd64-<tag>-<os>-<arch>-static` or `-shared`.

## Installer options

The install scripts download the latest GitHub Release for your platform, unpack it to `$HOME/bennugd`, set `BENNUGD_HOME`, and prepend it to `PATH`.

| Variable | Default | Description |
|----------|---------|-------------|
| `BENNUGD_HOME` | `$HOME/bennugd` | Install directory |
| `BENNUGD_VERSION` | `latest` | Release tag (for example `v1.0.0`) |
| `BENNUGD_LINKAGE` | `static` | `static` or `shared` archive to install |

## CI/CD

GitHub Actions (`.github/workflows/ci.yml`) builds `bgdc` and `bgdi` for:

- Linux x86_64
- Windows x86_64 (MinGW-w64 UCRT64 via MSYS2)
- macOS arm64

On tags matching `v*` (for example `v1.0.0`), the workflow publishes a GitHub Release with archives that embed zlib, libpng, SDL3, SDL3_mixer and the bundled DES library statically. OS graphics/audio libraries may still be required at runtime.
