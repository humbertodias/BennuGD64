# Building BennuGD64

## Dependencies

A default CMake configure **downloads and compiles** zlib, libpng, SDL3 and SDL3_mixer
(`BENNUGD_BUNDLE_DEPS=ON`). The first configure needs network access.

You still need a C compiler, CMake, Ninja (or Make), and OS development packages
that SDL3 links against (X11/Wayland/ALSA/Pulse on Linux; Xcode CLT on macOS;
MinGW-w64 on Windows).

Optional:

- OpenSSL, unless you set `USE_LIBDES` to use the bundled DES library
- System SDL3/zlib/libpng, if you pass `-DBENNUGD_BUNDLE_DEPS=OFF`

## Build

```shell
cmake -S . -B build -DUSE_LIBDES=ON
cmake --build build
```

Shared modules:

```shell
cmake -S . -B build-shared -DUSE_LIBDES=ON -DSTATIC_MODULES=OFF
cmake --build build-shared
```

Use system libraries instead of fetching them:

```shell
cmake -S . -B build -DBENNUGD_BUNDLE_DEPS=OFF
cmake --build build
```

Binaries:

| Tool | Path |
|------|------|
| bgdc | `build/core/bgdc/src/bgdc` |
| bgdi | `build/core/bgdi/src/bgdi` |

## Install / package

`cmake --install` writes a relocatable tree (`bgdc`, `bgdi`, `libbgdrtm` next to them, plugins in `modules/`):

```shell
cmake --install build --prefix dist/bennugd64
```

### CMake options

| Option | Default | Description |
|--------|---------|-------------|
| `BENNUGD_BUNDLE_DEPS` | ON | Fetch and build zlib, libpng, SDL3, SDL3_mixer |
| `BENNUGD_STATIC_DEPS` | OFF (ON when bundling) | Prefer static zlib/libpng/SDL3/SDL3_mixer |
| `BENNUGD_VERSION` | *(git tag)* | Banner, `BUILD_INFO.txt` and archive name. Empty = tag on HEAD (e.g. `1.2.3`), else `git describe` |
| `USE_LIBDES` | OFF | Use bundled DES instead of OpenSSL |
| `STATIC_MODULES` | ON | Link modules into `bgdi` (`OFF` builds shared `.so`/`.dll`) |
| `NO_SOUND` | OFF | Build without SDL3_mixer |

Release archives are named `bennugd64-<tag>-<os>-<arch>-static` or `-shared`.

## Installer options

The install scripts download the latest GitHub Release for your platform, unpack it to `$HOME/bennugd`, set `BENNUGD_HOME`, and prepend it to `PATH`.

| Variable | Default | Description |
|----------|---------|-------------|
| `BENNUGD_HOME` | `$HOME/bennugd` | Install directory |
| `BENNUGD_VERSION` | `latest` | Release tag (for example `1.2.3`) |
| `BENNUGD_LINKAGE` | `static` | `static` or `shared` archive to install |

## CI/CD

GitHub Actions (`.github/workflows/ci.yml`) builds `bgdc` and `bgdi` for:

- Linux x86_64
- Windows x86_64 (MinGW-w64 UCRT64 via MSYS2)
- macOS arm64

The workflow is CMake-only: `cmake -S/-B`, `cmake --build`, `cmake --install`.

On any git tag (for example `1.2.3`), that tag is the version in the `bgdc`/`bgdi` banners, `BUILD_INFO.txt`, and archive names. The workflow publishes a GitHub Release with archives that embed zlib, libpng, SDL3, SDL3_mixer and the bundled DES library statically. OS graphics/audio libraries may still be required at runtime.

