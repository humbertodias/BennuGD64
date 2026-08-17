# Building BennuGD64

## Docker (Linux, Windows, web, Android, and Switch)

No local compiler, CMake, MinGW, Emscripten, Android SDK, or devkitPro. Only [Docker](https://docs.docker.com/get-docker/).

```shell
bash scripts/docker-build.sh linux
bash scripts/docker-build.sh linux shared
bash scripts/docker-build.sh wasm
bash scripts/docker-build.sh android
bash scripts/docker-build.sh switch
bash scripts/docker-build.sh windows
bash scripts/docker-build.sh windows shared
```

| Command | Image | Output |
|---------|-------|--------|
| `linux` / `linux shared` | `docker/Dockerfile.linux` (`--target linux`) | `dist/linux-{static,shared}/` |
| `wasm` | `docker/Dockerfile.linux` (`--target wasm`) | `dist/web-wasm32-static/` |
| `android` | `docker/Dockerfile.android` | `dist/android-arm64-static/` (`bennugd64.apk`) |
| `switch` | `docker/Dockerfile.switch` | `dist/switch-aarch64-static/` (`bennugd64.nro`) |
| `windows` / `windows shared` | `docker/Dockerfile.windows` | `dist/windows-x86_64-{static,shared}/` |

The images are toolchains only. `scripts/docker-build.sh` builds the image, then `docker run` with the repo mounted and `cmake --preset` / `ctest --preset` (wasm: native `bgdc` + `emcmake` for `bgdi`; Android: native `bgdc` + NDK `libmain.so` + Gradle APK; Switch: native `bgdc` + libnx `bgdi.elf` + `elf2nro`). GitHub Actions uses the same Dockerfiles (`docker/build-push-action` + the same wrapper). The wasm stage is `FROM linux` in `Dockerfile.linux` (Emscripten SDK on the native toolchain).

```shell
bash scripts/docker-build.sh linux shell
bash scripts/docker-build.sh android shell
bash scripts/docker-build.sh switch shell
```

Zed and VS Code can attach to the same images via [Dev Containers](https://containers.dev/) (`.devcontainer/`). The default is the Linux toolchain; pick **Web (Emscripten)**, **Windows (MinGW)**, **Android (NDK)**, or **Switch (devkitA64)** in the config picker. The repo is mounted at `/src`, same as `docker-build.sh`.

macOS binaries cannot be produced from Linux containers (Apple SDK). Use a Mac or the `macos-latest` GitHub Actions job.

Pinned dependency versions live in `versions.env`. Native Linux packages are listed in `docker/Dockerfile.linux`.

With a local toolchain, `cmake --preset static` (or `make static`) configures, builds, installs, and runs CTest. On Darwin, `make` installs to `dist/macos-arm64-*`.

## Native CMake

A default CMake configure **downloads and compiles** zlib, libpng, SDL3 and SDL3_mixer
(`BENNUGD_BUNDLE_DEPS=ON`). The first configure needs network access.

You still need a C compiler, CMake 3.19+, Ninja, and OS development packages
that SDL3 links against (X11/Wayland/ALSA/Pulse on Linux; Xcode CLT on macOS;
MinGW-w64 on Windows). Presets in `CMakePresets.json`:

| Preset | Build dir | Notes |
|--------|-----------|--------|
| `static` / `shared` | `build-static` / `build-shared` | Host OS |
| `windows-static` / `windows-shared` | `build-windows-*` | MinGW-w64 from Linux |
| `wasi` | `build-wasi` | `bgdc.wasm` |
| `wasm-host` | `build-host` | Native `bgdc` for wasm demos |
| `android-host` | `build-android-host` | Native `bgdc` for Android demo DCBs |
| `android-arm64` | `build-android-arm64` | NDK `libmain.so` (needs `ANDROID_NDK`) |
| `switch-host` | `build-switch-host` | Native `bgdc` for Switch demo DCBs |
| `switch-aarch64` | `build-switch-aarch64` | libnx `bgdi.elf` (needs `DEVKITPRO`) |

Optional:

- OpenSSL, unless you set `USE_LIBDES` to use the bundled DES library
- System SDL3/zlib/libpng, if you pass `-DBENNUGD_BUNDLE_DEPS=OFF`

```shell
cmake --preset static
cmake --build --preset static
cmake --install build-static --prefix dist/bennugd64
ctest --preset static --output-on-failure
```

Shared modules: `--preset shared` (install prefix `dist/linux-shared` or `dist/macos-arm64-shared`).

Use system libraries instead of fetching them:

```shell
cmake --preset static -DBENNUGD_BUNDLE_DEPS=OFF
cmake --build --preset static
```

Binaries (static preset):

| Tool | Path |
|------|------|
| bgdc | `build-static/core/bgdc/src/bgdc` |
| bgdi | `build-static/core/bgdi/src/bgdi` |

CTest (`ctest --preset static`) checks the `bgdc`/`bgdi` banners and compiles `web/demo/hello.prg` to a `.dcb`. Shared presets skip the hello compile (modules live under `modules/` only after install). Windows cross-builds skip CTest (the `.exe` files cannot run on Linux).

## Version

By default CMake reads the version from Git:

- If `HEAD` is exactly a tag such as `1.2.3`, that tag is the version.
- Otherwise it uses `git describe --tags` (for example `1.2.3-4-gabc1234`).

That string is used in the `bgdc`/`bgdi` banners, the SDL window title, `BUILD_INFO.txt`, and archive names. Compiler macros (`__BGD__`, `__BGD_MINOR__`, `__BGD_PATCHLEVEL__`) take the `X.Y.Z` prefix (a leading `v` is ignored).

Override it at configure time:

```shell
cmake --preset static -DBENNUGD_VERSION=1.2.3
cmake --build --preset static
```

`scripts/docker-build.sh` passes `git describe` as `BENNUGD_VERSION` unless you set that variable yourself.

## Install / package

`cmake --install` writes a relocatable tree (`bgdc`, `bgdi`, `libbgdrtm` next to them, plugins in `modules/`):

```shell
cmake --install build-static --prefix dist/bennugd64
```

Docker builds already install into `dist/<target>/`.

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
| `INTERPRETER_ONLY` | OFF | Build only `bgdi` (no `bgdc`). Used by the Emscripten and Android player builds |
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
cmake --preset wasi
cmake --build --preset wasi
cmake --install build-wasi --prefix dist/wasi-wasm32-static
ctest --preset wasi --output-on-failure
```

The binary is `bgdc.wasm`. This build is compiler-only: zlib + bundled DES, no SDL, no `bgdi`.

Run it with [Wasmtime](https://wasmtime.dev/). `--dir=.` preopens the current directory so the module can read `.prg` files and write the `.dcb`. Paths must stay under a preopened directory (a host `/tmp` is not visible unless you also pass `--dir=/tmp`):

```shell
wasmtime --dir=. ./dist/wasi-wasm32-static/bgdc.wasm -- -o web/demo/hello.dcb web/demo/hello.prg
```

You can also pass an existing toolchain without `BENNUGD_WASI`:

```shell
cmake -B build-wasi \
  -DCMAKE_TOOLCHAIN_FILE="$WASI_SDK_PATH/share/cmake/wasi-sdk-p1.cmake" \
  -DUSE_LIBDES=ON
```

### Emscripten interpreter (`bgdi`)

Needs Docker. `Dockerfile.linux` `--target wasm` installs the [Emscripten SDK](https://emscripten.org/) on the Linux stage (native `bgdc` + `emcmake` for `bgdi`):

```shell
bash scripts/docker-build.sh wasm
python3 -m http.server 8080 --directory dist/web-wasm32-static
```

Open `http://localhost:8080/`. Drop a `.dcb` plus assets (or a game folder) onto the page to run it. Every `web/demo/*.dcb` is preloaded and listed as a bundled demo. Asyncify lets `SDL_Delay` yield to the browser.

Pushes to `main` run CI, then `.github/workflows/pages.yml` publishes the `web-wasm32-static` artifact as `index.html`. In the repo set **Settings → Pages → Source** to **GitHub Actions**.

## Android

Needs Docker. `docker/Dockerfile.android` is a toolchain image (JDK 17, Android SDK/NDK, CMake, Ninja, Gradle) built for **linux/amd64** (the NDK host tools are x86_64; on Apple Silicon Docker uses qemu). It does not clone this repo or bake Bennu into the image.

```shell
bash scripts/docker-build.sh android
```

That configures native `bgdc` (`android-host`), compiles `web/demo/*.prg`, cross-compiles `libmain.so` with the NDK (`android-arm64`, API 28, arm64-v8a), copies SDL3 Java from FetchContent, and runs Gradle to produce `dist/android-arm64-static/bennugd64.apk`.

Install the debug APK on an arm64 device or emulator (Android 9+). The APK ships `hello.dcb` as `assets/main.dcb`. To ship your own game, replace `assets/main.dcb` (and any extra files) in a copy of `android/` or add them under the app files directory.

SDL3 is a shared `libSDL3.so` because `SDLActivity` loads it next to `libmain.so`. Modules are linked into `libmain.so`.

## Nintendo Switch

Needs Docker. `docker/Dockerfile.switch` is a toolchain image (`devkitpro/devkita64` plus host gcc, CMake, Ninja) built for **linux/amd64** (devkitA64 host tools are x86_64; on Apple Silicon Docker uses qemu). It does not clone this repo or bake Bennu into the image.

```shell
bash scripts/docker-build.sh switch
```

That configures native `bgdc` (`switch-host`), compiles `web/demo/*.prg`, cross-compiles `bgdi.elf` with libnx (`switch-aarch64`), and runs `nacptool` / `elf2nro` to produce `dist/switch-aarch64-static/bennugd64.nro`. The NRO RomFS ships `hello.dcb` as `main.dcb`. Copy the `.nro` to `sdmc:/switch/` on a homebrew Switch, or send it with `nxlink`. Extra files can go next to the NRO or under `sdmc:/switch/bennugd64`.

SDL3 comes from the [devkitPro SDL `switch-sdl-3.4`](https://github.com/devkitPro/SDL/tree/switch-sdl-3.4) branch (homebrew video/audio). Modules are linked into `bgdi.elf`.

## Installer options

The install scripts download the latest GitHub Release for your platform, unpack it to `$HOME/bennugd`, set `BENNUGD_HOME`, and prepend it to `PATH`.

| Variable | Default | Description |
|----------|---------|-------------|
| `BENNUGD_HOME` | `$HOME/bennugd` | Install directory |
| `BENNUGD_VERSION` | `latest` | Release tag (for example `1.2.3`) |
| `BENNUGD_LINKAGE` | `static` | `static` or `shared` archive to install |

## CI/CD

GitHub Actions (`.github/workflows/ci.yml`):

- Linux x86_64 and Windows x86_64 — one toolchain image per OS, then static + shared (`scripts/docker-build.sh`)
- macOS arm64 — native runner (static + shared in one job)
- Web (Emscripten) — `scripts/docker-build.sh wasm` (`Dockerfile.linux --target wasm`) → `bennugd64-<tag>-web-wasm32-static.zip`; Pages deploys this artifact
- Android arm64 — `scripts/docker-build.sh android` (`Dockerfile.android`) → `bennugd64-<tag>-android-arm64-static.zip` (`bennugd64.apk`)
- Nintendo Switch — `scripts/docker-build.sh switch` (`Dockerfile.switch`) → `bennugd64-<tag>-switch-aarch64-static.zip` (`bennugd64.nro`)
- WASI — `cmake --preset wasi` + CTest (Wasmtime) → `bennugd64-<tag>-wasi-wasm32-static.zip`

On any git tag (for example `1.2.3`), that tag is the version in the `bgdc`/`bgdi` banners, `BUILD_INFO.txt`, and archive names (`bennugd64-<tag>-<os>-<arch>-static` or `-shared`; wasm zips for `web-wasm32-static` and `wasi-wasm32-static`; Android zip for `android-arm64-static`; Switch zip for `switch-aarch64-static`). The workflow publishes a GitHub Release with archives that embed zlib, libpng, SDL3, SDL3_mixer and the bundled DES library statically (WASI archives embed only zlib and DES; the Android APK ships shared `libSDL3.so` + `libmain.so`; the Switch NRO links SDL3 statically from the devkitPro fork). OS graphics/audio libraries may still be required at runtime on native builds.
