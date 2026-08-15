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

## Version

By default CMake reads the version from Git:

- If `HEAD` is exactly a tag such as `1.2.3`, that tag is the version.
- Otherwise it uses `git describe --tags` (for example `1.2.3-4-gabc1234`).

That string is used in the `bgdc`/`bgdi` banners, the SDL window title, `BUILD_INFO.txt`, and archive names. Compiler macros (`__BGD__`, `__BGD_MINOR__`, `__BGD_PATCHLEVEL__`) take the `X.Y.Z` prefix (a leading `v` is ignored).

Override it at configure time:

```shell
cmake -S . -B build -DUSE_LIBDES=ON -DBENNUGD_VERSION=1.2.3
cmake --build build
```

`make static` / `make shared` follow the same rule unless you pass the flag through `CMAKE_FLAGS`:

```shell
make static CMAKE_FLAGS="-DUSE_LIBDES=ON -DBENNUGD_VERSION=1.2.3"
```

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
| `BENNUGD_VERSION` | *(git tag)* | Override the version (see [Version](#version)); empty = tag on HEAD or `git describe` |
| `USE_LIBDES` | OFF | Use bundled DES instead of OpenSSL |
| `STATIC_MODULES` | ON | Link modules into `bgdi` (`OFF` builds shared `.so`/`.dll`) |
| `NO_SOUND` | OFF | Build without SDL3_mixer |
| `COMPILER_ONLY` | OFF | Build only `bgdc` (no `bgdi` / modules). Forced ON for WASI |
| `INTERPRETER_ONLY` | OFF | Build only `bgdi` (no `bgdc`). Used by the Emscripten player build |
| `BENNUGD_WASI` | OFF | Cross-compile `bgdc` to wasm32-wasip1 (downloads wasi-sdk if needed) |
| `BENNUGD_WASI_SDK_VERSION` | `33` | wasi-sdk major version to fetch when `BENNUGD_WASI=ON` |
| `BENNUGD_WASI_SDK_VERSION_FULL` | `33.0` | wasi-sdk full version (tarball name) |

## WebAssembly

There are two wasm targets. They use different toolchains and are not interchangeable:

| Artifact | Toolchain | What it is |
|----------|-----------|------------|
| `bgdc.wasm` | [wasi-sdk](https://github.com/WebAssembly/wasi-sdk) (WASI p1) | Cross-platform **compiler**. Run with Wasmtime (or any WASI runtime). |
| `bgdi.html` / `bgdi.wasm` | [Emscripten](https://emscripten.org/) | Browser **interpreter**. Needs a `.dcb` compiled ahead of time. |

### WASI compiler (`bgdc.wasm`)

CMake fetches [wasi-sdk](https://github.com/WebAssembly/wasi-sdk) into `.deps/` on the first configure (needs network). If `WASI_SDK_PATH` already points at an SDK with `bin/clang`, that install is used instead.

```shell
cmake -B build-wasi -DBENNUGD_WASI=ON
cmake --build build-wasi --target bgdc
cmake --install build-wasi --prefix dist/wasi
```

The binary is `build-wasi/core/bgdc/src/bgdc.wasm` (and `dist/wasi/bgdc.wasm` after install). This build is compiler-only: zlib + bundled DES, no SDL, no `bgdi`.

Run it with [Wasmtime](https://wasmtime.dev/) (`brew install wasmtime` on macOS). `--dir=.` preopens the current directory so the module can read `.prg` files and write the `.dcb`. Paths must stay under a preopened directory (a host `/tmp` is not visible unless you also pass `--dir=/tmp`):

```shell
wasmtime --dir=. ./dist/wasi/bgdc.wasm -- -o web/demo/hello.dcb web/demo/hello.prg
```

You can also pass an existing toolchain without `BENNUGD_WASI`:

```shell
cmake -B build-wasi \
  -DCMAKE_TOOLCHAIN_FILE="$WASI_SDK_PATH/share/cmake/wasi-sdk-p1.cmake" \
  -DUSE_LIBDES=ON
```

### Emscripten interpreter (`bgdi`)

Needs the [Emscripten SDK](https://emscripten.org/) (`emcmake` on `PATH`). Compile demo DCBs with native `bgdc`, then:

```shell
make static
make wasm CMAKE_FLAGS="-DUSE_LIBDES=ON"
python3 -m http.server 8080 --directory build-wasm/core/bgdi/src
```

Open `http://localhost:8080/bgdi.html`. Drop a `.dcb` plus assets (or a game folder) onto the page to run it. Every `web/demo/*.dcb` is preloaded and listed as a bundled demo. Asyncify lets `SDL_Delay` yield to the browser.

Pushes to `main` run `.github/workflows/pages.yml`: it compiles `bgdc`, every `web/demo/*.prg`, then `bgdi` for wasm32 and publishes `bgdi.html` as `index.html`. In the repo set **Settings → Pages → Source** to **GitHub Actions**.

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
- Web (Emscripten): `bennugd64-<tag>-web-wasm32-static.zip` — browser interpreter (`index.html`, `bgdi.js`, `bgdi.wasm`, `bgdi.data`)
- WASI: `bennugd64-<tag>-wasi-wasm32-static.zip` — `bgdc.wasm`, smoke-tested with Wasmtime

The workflow is CMake-only: `cmake -S/-B`, `cmake --build`, `cmake --install`. The WASI job uses `-DBENNUGD_WASI=ON` (same path as a local `build-wasi`).

On any git tag (for example `1.2.3`), that tag is the version in the `bgdc`/`bgdi` banners, `BUILD_INFO.txt`, and archive names (`bennugd64-<tag>-<os>-<arch>-static` or `-shared`; wasm zips for `web-wasm32-static` and `wasi-wasm32-static`). The workflow publishes a GitHub Release with archives that embed zlib, libpng, SDL3, SDL3_mixer and the bundled DES library statically (WASI archives embed only zlib and DES). OS graphics/audio libraries may still be required at runtime on native builds.

