# Building BennuGD64

## Docker (Linux, Windows, web, Android, Switch, Dreamcast, PSP, Vita, PS2, Pandora, Wii, and macOS)

No local compiler, CMake, MinGW, Emscripten, Android SDK, devkitPro, KallistiOS, pspdev, vitasdk, ps2dev, Ångström toolchain, or Xcode. Only [Docker](https://www.docker.com/get-started/).

```shell
bash scripts/build.sh linux
bash scripts/build.sh linux shared
bash scripts/build.sh wasm
bash scripts/build.sh android
bash scripts/build.sh switch
bash scripts/build.sh dreamcast
bash scripts/build.sh psp
bash scripts/build.sh vita
bash scripts/build.sh ps2
bash scripts/build.sh pandora
bash scripts/build.sh wii
bash scripts/build.sh macos
bash scripts/build.sh macos arm64
bash scripts/build.sh macos arm64 shared
bash scripts/build.sh windows
bash scripts/build.sh windows shared
```

| Command | Image | Output |
|---------|-------|--------|
| `linux` / `linux shared` | `docker/Dockerfile.linux` (`--target linux`) | `dist/linux-{static,shared}/` |
| `wasm` | `docker/Dockerfile.wasm` | `dist/web-wasm32-static/` |
| `android` | `docker/Dockerfile.android` | `dist/android-arm64-static/` (`bennugd64.apk`) |
| `switch` | `docker/Dockerfile.switch` | `dist/switch-aarch64-static/` (`bennugd64.nro`) |
| `dreamcast` | `docker/Dockerfile.dreamcast` | `dist/dreamcast-sh4-static/` (`bennugd64.cdi`) |
| `psp` | `docker/Dockerfile.psp` | `dist/psp-mips-static/` (`EBOOT.PBP`) |
| `vita` | `docker/Dockerfile.vita` | `dist/vita-arm-static/` (`bennugd64.vpk`) |
| `ps2` | `docker/Dockerfile.ps2` | `dist/ps2-mips-static/` (`bgdi.elf`, `bennugd64.iso`) |
| `pandora` | `docker/Dockerfile.pandora` | `dist/pandora-arm-static/` (`bennugd64.pnd`) |
| `wii` | `docker/Dockerfile.wii` | `dist/wii-powerpc-static/` (`apps/bennugd64/boot.dol`) |
| `macos` / `macos shared` | `docker/Dockerfile.macos` | `dist/macos-x86_64-{static,shared}/` |
| `macos arm64` / `macos arm64 shared` | `docker/Dockerfile.macos` | `dist/macos-arm64-{static,shared}/` |
| `windows` / `windows shared` | `docker/Dockerfile.windows` | `dist/windows-x86_64-{static,shared}/` |

The images are toolchains only. `scripts/build.sh` builds the image, then `docker run` with the repo mounted and `cmake --preset` / `ctest --preset` (wasm: native `bgdc` + `emcmake` for `bgdi`; Android: native `bgdc` + NDK `libmain.so` + Gradle APK; Switch: native `bgdc` + libnx `bgdi.elf` + `elf2nro`; Dreamcast: native `bgdc` + KallistiOS `bgdi.elf` + `mkdcdisc`; PSP: native `bgdc` + pspdev `bgdi.elf` + `pack-pbp`; Vita: native `bgdc` + vitasdk `bgdi` + `vita-pack-vpk`; PS2: native `bgdc` + ps2dev `bgdi.elf`; Pandora: native `bgdc` + Ångström `bgdi` + `mksquashfs`; Wii: native `bgdc` + libogc `bgdi.elf` + `elf2dol`). GitHub Actions uses the same Dockerfiles (`docker/build-push-action` + the same wrapper). Wasm native `bgdc` is `COMPILER_ONLY`.

```shell
bash scripts/build.sh linux shell
bash scripts/build.sh android shell
bash scripts/build.sh switch shell
bash scripts/build.sh dreamcast shell
bash scripts/build.sh psp shell
bash scripts/build.sh vita shell
bash scripts/build.sh ps2 shell
bash scripts/build.sh pandora shell
bash scripts/build.sh wii shell
bash scripts/build.sh macos shell
```

Zed and VS Code can attach to the same images via [Dev Containers](https://containers.dev/) (`.devcontainer/`). The default is the Linux toolchain; pick **Web (Emscripten)**, **Windows (MinGW)**, **Android (NDK)**, **Switch (devkitA64)**, **Dreamcast (KallistiOS)**, **PSP (pspdev)**, **PS Vita (vitasdk)**, **PS2 (ps2dev)**, **OpenPandora (Ångström)**, **Wii (devkitPPC)**, or **macOS (osxcross)** in the config picker. The repo is mounted at `/src`, same as `build.sh`.

The first `macos` image build downloads a macOS SDK (Xcode license) and compiles [osxcross](https://github.com/tpoechtrager/osxcross); later runs reuse `bennugd64-macos`. SDL3 is built from FetchContent (not `osxcross-macports` / SDL2). SDK 10.10 is too old for SDL3; the image uses MacOSX 14.5 with deployment target 11.0.

Docker osxcross is the Linux → Mach-O path (`dist/macos-x86_64-*` or `dist/macos-arm64-*`). Shared macOS archives from the internet are quarantined; dyld then refuses `@rpath` dylibs. Run `./unquarantine.sh` from the extracted folder (it `xattr -cr`s the directory and execs `./bgdi`). On a copy that still fails: `codesign --force --sign - ./bgdi ./bgdc ./*.dylib ./modules/*.dylib`.

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
| `dreamcast-host` | `build-dreamcast-host` | Native `bgdc` for Dreamcast demo DCBs |
| `dreamcast-sh4` | `build-dreamcast-sh4` | KallistiOS `bgdi.elf` (needs `KOS_BASE`) |
| `psp-host` | `build-psp-host` | Native `bgdc` for PSP demo DCBs |
| `psp-mips` | `build-psp-mips` | pspdev `bgdi.elf` (needs `PSPDEV`) |
| `vita-host` | `build-vita-host` | Native `bgdc` for Vita demo DCBs |
| `vita-arm` | `build-vita-arm` | vitasdk `bgdi` (needs `VITASDK`) |
| `ps2-host` | `build-ps2-host` | Native `bgdc` for PS2 demo DCBs |
| `ps2-mips` | `build-ps2-mips` | ps2dev `bgdi.elf` (needs `PS2DEV`) |
| `pandora-host` | `build-pandora-host` | Native `bgdc` for Pandora demo DCBs |
| `pandora-arm` | `build-pandora-arm` | Ångström `bgdi` (needs `TOOLCHAIN=/opt/openpandora`) |
| `wii-host` | `build-wii-host` | Native `bgdc` for Wii demo DCBs |
| `wii-powerpc` | `build-wii-powerpc` | libogc `bgdi.elf` (needs `DEVKITPRO`) |
| `macos-x86_64-static` / `macos-x86_64-shared` | `build-macos-x86_64-*` | osxcross `o64-clang` |
| `macos-arm64-static` / `macos-arm64-shared` | `build-macos-arm64-*` | osxcross `oa64-clang` |

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

`scripts/build.sh` passes `git describe` as `BENNUGD_VERSION` unless you set that variable yourself.

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

Needs Docker. `docker/Dockerfile.wasm` extends [`emscripten/emsdk`](https://hub.docker.com/r/emscripten/emsdk) with ninja (native `bgdc` is compiler-only; `emcmake` builds `bgdi`):

```shell
bash scripts/build.sh wasm
python3 -m http.server 8080 --directory dist/web-wasm32-static
```

Open `http://localhost:8080/`. Drop a `.dcb` plus assets (or a game folder) onto the page to run it. Every `web/demo/*.dcb` is preloaded and listed as a bundled demo. Asyncify lets `SDL_Delay` yield to the browser.

Pushes to `main` run CI, then `.github/workflows/pages.yml` publishes the `web-wasm32-static` artifact as the site root and Doxygen API docs under `/docs/` (see `doc/pages.yml`). In the repo set **Settings → Pages → Source** to **GitHub Actions**.

Generate API docs locally (Docker image `bennugd64-doxygen`, or local `doxygen` if installed):

```shell
bash doc/generate.sh
# open doc/html/index.html
```

Force the container (same image as GitHub Pages):

```shell
USE_DOCKER=1 bash doc/generate.sh
```

## Android

Needs Docker. `docker/Dockerfile.android` is a toolchain image (JDK 17, Android SDK/NDK, CMake, Ninja, Gradle) built for **linux/amd64** (the NDK host tools are x86_64; on Apple Silicon Docker uses qemu). It does not clone this repo or bake Bennu into the image.

```shell
bash scripts/build.sh android
```

That configures native `bgdc` (`android-host`), compiles `web/demo/*.prg`, cross-compiles `libmain.so` with the NDK (`android-arm64`, API 28, arm64-v8a), copies SDL3 Java from FetchContent, and runs Gradle to produce `dist/android-arm64-static/bennugd64.apk`.

Install the debug APK on an arm64 device or emulator (Android 9+). The APK ships `hello.dcb` as `assets/main.dcb`. To ship your own game, replace `assets/main.dcb` (and any extra files) in a copy of `android/` or add them under the app files directory.

SDL3 is a shared `libSDL3.so` because `SDLActivity` loads it next to `libmain.so`. Modules are linked into `libmain.so`.

## Nintendo Switch

Needs Docker. `docker/Dockerfile.switch` is a toolchain image (`devkitpro/devkita64` plus host gcc, CMake, Ninja) built for **linux/amd64** (devkitA64 host tools are x86_64; on Apple Silicon Docker uses qemu). It does not clone this repo or bake Bennu into the image.

```shell
bash scripts/build.sh switch
```

That configures native `bgdc` (`switch-host`), compiles `web/demo/*.prg`, cross-compiles `bgdi.elf` with libnx (`switch-aarch64`), and runs `nacptool` / `elf2nro` to produce `dist/switch-aarch64-static/bennugd64.nro`. The NRO RomFS ships `hello.dcb` as `main.dcb`. Copy the `.nro` to `sdmc:/switch/` on a homebrew Switch, or send it with `nxlink`. Extra files can go next to the NRO or under `sdmc:/switch/bennugd64`.

SDL3 comes from the [devkitPro SDL `switch-sdl-3.4`](https://github.com/devkitPro/SDL/tree/switch-sdl-3.4) branch (homebrew video/audio). Modules are linked into `bgdi.elf`.

## Sega Dreamcast

Needs Docker. `docker/Dockerfile.dreamcast` is a toolchain image built for **linux/amd64** (SH4 host tools are x86_64; on Apple Silicon Docker uses qemu). It does not clone this repo or bake Bennu into the image.

The toolchain image is [`kallistios/dc-kos-toolchain`](https://hub.docker.com/r/kallistios/dc-kos-toolchain) (KallistiOS, `sh-elf` GCC 14, CMake, Ninja, `mkdcdisc`). `mkdcdisc` writes IP.BIN; the old `nold360/kallistios-sdk` Hub image is not used (Buildx cannot COPY layers from that 2021 image).

```shell
bash scripts/build.sh dreamcast
```

That configures native `bgdc` (`dreamcast-host`), compiles `web/demo/*.prg`, cross-compiles `bgdi.elf` with KallistiOS (`dreamcast-sh4`), and runs `mkdcdisc` to produce `dist/dreamcast-sh4-static/bennugd64.cdi`. The ISO ships `hello.dcb` as `main.dcb` (loaded from `/cd/`). Burn or emulate the `.cdi`, or send `bgdi.elf` with `dc-tool`. Extra files can go on the ISO next to `main.dcb`.

SDL3 comes from the [GPF SDL `dreamcastSDL3`](https://github.com/GPF/SDL/tree/dreamcastSDL3) branch (KallistiOS video/audio). Modules are linked into `bgdi.elf`.

## PlayStation Portable

Needs Docker. `docker/Dockerfile.psp` is a toolchain image built for **linux/amd64** (psp-gcc host tools are x86_64; on Apple Silicon Docker uses qemu). It does not clone this repo or bake Bennu into the image.

The toolchain image is [`pspdev/pspdev`](https://hub.docker.com/r/pspdev/pspdev) (`psp-gcc` GCC 15, pspsdk, CMake, packaged SDL3, `pack-pbp` / `mksfoex`). Host gcc and Ninja are installed for native `bgdc`.

```shell
bash scripts/build.sh psp
```

That configures native `bgdc` (`psp-host`), compiles `web/demo/*.prg`, cross-compiles `bgdi.elf` with pspdev (`psp-mips`), and runs `pack-pbp` to produce `dist/psp-mips-static/EBOOT.PBP`. The game folder ships `hello.dcb` as `main.dcb`. Copy the folder to `ms0:/PSP/GAME/bennugd64/` on a homebrew PSP (or open `EBOOT.PBP` in PPSSPP). Put your game's `main.dcb` next to `EBOOT.PBP`.

For **Streets of Rage Remake**, copy the **entire** SoRR install folder (not just the DCB): `mod/`, `palettes/`, etc., plus `SorR.dat` or `main.dcb` beside `EBOOT.PBP`. In `mod/system.txt`, set the first line after the comments to `PSP` (not `PC`) so menus and controls match handheld ports. SoRR is a large game (~300 MB DCB); it may run in PPSSPP on a PC but can run out of memory on real PSP hardware.

SDL3 and SDL3_mixer come from the pspdev packages (Allegrex GU/audio). Modules are linked into `bgdi.elf`.

## PlayStation Vita

Needs Docker. `docker/Dockerfile.vita` is a toolchain image from [`vitasdk/vitasdk`](https://hub.docker.com/r/vitasdk/vitasdk) (Ubuntu 24.04, `arm-vita-eabi-gcc`, `vita-elf-create` / `vita-make-fself` / `vita-pack-vpk`). Host gcc and Ninja are installed for native `bgdc`. The image is linux/amd64 and linux/arm64.

```shell
bash scripts/build.sh vita
```

That configures native `bgdc` (`vita-host`), compiles `web/demo/*.prg`, cross-compiles `bgdi` with vitasdk (`vita-arm`), and packs `dist/vita-arm-static/bennugd64.vpk` (title id `BGDV00001`). Install the VPK on a jailbroken Vita.

**Where `main.dcb` goes** (first match wins):

1. `ux0:/data/bennugd64/main.dcb` — drop-in game (create the folder in VitaShell; name must be `main.dcb`)
2. `app0:/main.dcb` — bundled in the VPK (`hello.dcb` from `web/demo/hello.prg`)
3. `app0:/` cwd as `main.dcb`

Other assets (FPG, WAV, palettes, …) also go in `ux0:/data/bennugd64/` or next to the DCB in the VPK. Compile with the matching `vita-host` `bgdc`, not a random desktop Bennu.

On **real hardware**, relative `fopen` does not follow POSIX cwd the way Vita3K does. The interpreter prefixes asset opens with the directory that held `main.dcb` (`ux0:/data/bennugd64/` or `app0:/`) so both the emulator and a device see the same files.

For **Streets of Rage Remake**, copy the **entire** install into `ux0:/data/bennugd64/` (not only the DCB): `palettes/`, `mod/`, maps, and so on, plus `main.dcb` (rename `SorR.dat` if needed). In `mod/system.txt`, handheld ports use `PSP`. The interpreter keeps SoRR's 16-bit buffer (GXM uploads RGB565), turns vsync off, and sets ARM/bus/GPU to 444/222/222; Vita3K will still look smooth, the device no longer software-scales to 960x544.

If none of those files exist, the screen lists the paths (same idea as the PS2 missing-DCB help). CROSS / START / SELECT quits. LiveArea has no console; copy `ux0:/data/bennugd64/bgdi.log` after a silent close. SELECT is Escape (hello's `key(_esc)`); START is Enter.

Crashes write `ux0:/data/psp2core-*-eboot.bin.psp2dmp` (gzip ELF core). Copy it off the Vita and:

```shell
python3 scripts/vita/parse-dmp.py ~/psp2core-….psp2dmp dist/vita-arm-static/bgdi.elf
```

Use the unstripped `bgdi.elf` from the same build, not `eboot.bin`. For disassembly around PC: [vita-parse-core](https://github.com/xyzz/vita-parse-core) (`python2 main.py dump.psp2dmp bgdi.elf`) with `arm-vita-eabi-addr2line` on `PATH`.

SDL3 is FetchContent (official Vita GXM backend). Modules are linked into `bgdi`.

## PlayStation 2

Needs Docker. `docker/Dockerfile.ps2` is a toolchain image built for **linux/amd64** (Emotion Engine host tools are x86_64; on Apple Silicon Docker uses qemu). It does not clone this repo or bake Bennu into the image.

The toolchain image is [`ps2dev/ps2dev`](https://hub.docker.com/r/ps2dev/ps2dev) (`mips64r5900el-ps2-elf-gcc`, ps2sdk, gsKit, `bin2c`). Host gcc and Ninja are installed for native `bgdc`. USB `usbd.irx` / `usbhdfsd.irx` are embedded like the historical BennuGD PS2 port.

```shell
bash scripts/build.sh ps2
```

That configures native `bgdc` (`ps2-host`), compiles `web/demo/*.prg`, cross-compiles `bgdi.elf` with ps2dev (`ps2-mips`), and stages `dist/ps2-mips-static/` (`bgdi.elf`, `main.dcb`, `SYSTEM.CNF`, `bennugd64.iso`).

**Where `main.dcb` goes:** always in the **same folder as `bgdi.elf`**. The Docker build already copies `hello.dcb` there as `main.dcb`. The ISO has the same files as ISO 9660 `MAIN.DCB` / `BGDI.ELF`.

PCSX2 (ISO / cdrom):

1. **File → Open** `dist/ps2-mips-static/bennugd64.iso` (or put the ISO in the disc tray and boot it).
2. The interpreter keeps CDVD mounted (no IOP reset), loads `cdfs:` and reads `cdrom0:\MAIN.DCB;1`.

PCSX2 (Host Filesystem — no USB image needed):

1. Leave `bgdi.elf` and `main.dcb` together in `dist/ps2-mips-static/` (do not open a copy of the ELF from another directory).
2. **Settings → Emulation → Enable Host Filesystem.** The interpreter keeps `host:` mounted (no IOP reset) and loads `host:main.dcb`. Without HostFS, `host:` is empty and you get a message asking for `main.dcb`.
3. **File → Open** `bgdi.elf` (not the ISO).

uLaunchELF on USB: copy the whole folder to the stick and run `bgdi.elf`; that path resets IOP and looks for `mass:/MAIN.DCB`. Put your game's `main.dcb` next to the ELF.

SDL3 is the official PS2 gsKit backend (FetchContent). zlib and libpng come from ps2sdk-ports so they match the headers already on the EE include path. Modules are linked into `bgdi.elf`.

## OpenPandora

Needs Docker. `docker/Dockerfile.pandora` is a toolchain image built for **linux/amd64** (Ångström host tools are x86_64; on Apple Silicon Docker uses qemu). It does not clone this repo or bake Bennu into the image.

The toolchain image is [`scummvm/dockerized-toolchains:openpandora`](https://hub.docker.com/r/scummvm/dockerized-toolchains) (`arm-angstrom-linux-gnueabi` GCC 8.3, glibc 2.9 sysroot, CMake, Ninja).

```shell
bash scripts/build.sh pandora
```

That configures native `bgdc` (`pandora-host`), compiles `web/demo/*.prg`, cross-compiles `bgdi` for Cortex-A8 (`pandora-arm`), and runs `mksquashfs` to produce `dist/pandora-arm-static/bennugd64.pnd`. The PND ships `hello.dcb` as `main.dcb`. Copy the `.pnd` to the Pandora SD card, or run `./bgdi` next to `main.dcb`. Extra files can go in the PND next to `main.dcb`.

SDL3 is the official Linux X11 software backend (the Ångström sysroot has no GLES). Modules are linked into `bgdi`.

## Nintendo Wii

Needs Docker. `docker/Dockerfile.wii` is a toolchain image (`devkitpro/devkitppc` plus host gcc, CMake, Ninja) built for **linux/amd64** (devkitPPC host tools are x86_64; on Apple Silicon Docker uses qemu). It does not clone this repo or bake Bennu into the image.

```shell
bash scripts/build.sh wii
```

That configures native `bgdc` (`wii-host`), compiles `web/demo/*.prg`, cross-compiles `bgdi.elf` with libogc (`wii-powerpc`), and runs `elf2dol` to produce `dist/wii-powerpc-static/apps/bennugd64/boot.dol`. The folder ships `hello.dcb` as `main.dcb`. Copy `apps/bennugd64/` to a real SD card and launch from the Homebrew Channel. Put your game's `main.dcb` next to `boot.dol`.

In Dolphin, **File → Open** `dist/wii-powerpc-static/bgdi.elf` (not the game list, and not `boot.dol` from the SD). Opening a DOL/ELF that lives **on** the virtual SD is a black screen; the interpreter must stay on the host disk, and `main.dcb` must be on the SD image. Do **not** point **SD Card Path** at a folder — that path is the `.raw` file, and using a directory makes Dolphin report **Failed to init core**. Use **Options → Configuration → Wii**:

1. Leave **SD Card Path** as the default `WiiSD.raw` (macOS: `~/Library/Application Support/Dolphin/Load/WiiSD.raw`).
2. Enable **Insert SD Card**.
3. Set **SD Sync Folder** to Dolphin's `Load/WiiSDSync` (macOS: `~/Library/Application Support/Dolphin/Load/WiiSDSync`). Put `main.dcb` in that folder root (`sd:/main.dcb`). Do not put `bgdi.elf` or `boot.dol` there.
4. Click **Convert Folder to File Now**. Leave **Automatically Sync with Folder** off while you run — converting an empty folder wipes the SD image.
5. **File → Open** `dist/wii-powerpc-static/bgdi.elf`.

The interpreter reads `sd:/main.dcb` from **`WiiSD.raw`**, not from `WiiSDSync` and not from `dist/`. The sync folder is only a staging area. Files move into the image when you click **Convert Folder to File Now** (or when automatic sync is on). Deleting `WiiSDSync` does not empty the `.raw`.

### Update only the DCB

Compile on the host (`bgdc -o main.dcb game.prg`). Then either:

**Mount the image** (replaces one file, leaves the rest of the card). Quit Dolphin first.

macOS:

```shell
hdiutil attach -imagekey diskimage-class=CRawDiskImage ~/Library/Application\ Support/Dolphin/Load/WiiSD.raw
```

The volume appears in Finder (often named **NO NAME**). Replace `main.dcb` at the root, then eject before opening Dolphin again:

```shell
hdiutil detach "/Volumes/NO NAME"
```

If attach reports *no mountable file systems*:

```shell
hdiutil mount "$(hdiutil attach -nomount -imagekey diskimage-class=CRawDiskImage ~/Library/Application\ Support/Dolphin/Load/WiiSD.raw)"
```

**Or use Convert:** **Convert File to Folder Now** (extracts `WiiSD.raw` into `WiiSDSync/`), replace `WiiSDSync/main.dcb`, then **Convert Folder to File Now**. Do not convert from an empty folder.

For **Streets of Rage Remake**, copy the **entire** install onto the SD image root (not only the DCB): `palettes/`, `mod/`, maps, and so on, plus `SorR.dat` or `main.dcb`. The interpreter adds those subfolders to the file PATH (SoRR loads `galsia.pal` by basename). In `mod/system.txt`, handheld ports use `PSP`; on Wii try `PC` if menus look wrong.

SDL3 comes from the [lucaspcamargo/SDL3-libogc2](https://github.com/lucaspcamargo/SDL3-libogc2) `fixes` branch (GX video / AESND audio). Modules are linked into `bgdi.elf`.

## Installer options

The install scripts download the latest GitHub Release for your platform, unpack it to `$HOME/bennugd`, set `BENNUGD_HOME`, and prepend it to `PATH`.

| Variable | Default | Description |
|----------|---------|-------------|
| `BENNUGD_HOME` | `$HOME/bennugd` | Install directory |
| `BENNUGD_VERSION` | `latest` | Release tag (for example `1.2.3`) |
| `BENNUGD_LINKAGE` | `static` | `static` or `shared` archive to install |

## CI/CD

GitHub Actions (`.github/workflows/ci.yml`):

- Docker platforms share one `build` job keyed by `matrix.platform` (`linux`, `windows`, `macos`/osxcross, `wasm`, `android`, `switch`, `dreamcast`, `psp`, `vita`, `ps2`, `pandora`, `wii`): `docker/Dockerfile.$platform` then `scripts/build.sh`. Linux, Windows, and osxcross (x86_64 and arm64) also build shared modules. Pages deploys the `web-wasm32-static` artifact.
- WASI — host `cmake --preset wasi` + CTest (Wasmtime) → `bennugd64-<tag>-wasi-wasm32-static.zip`

On any git tag (for example `1.2.3`), that tag is the version in the `bgdc`/`bgdi` banners, `BUILD_INFO.txt`, and archive names (`bennugd64-<tag>-<os>-<arch>-static` or `-shared`; wasm zips for `web-wasm32-static` and `wasi-wasm32-static`; Android zip for `android-arm64-static`; Switch zip for `switch-aarch64-static`; Dreamcast zip for `dreamcast-sh4-static`; PSP zip for `psp-mips-static`; Vita zip for `vita-arm-static`; Pandora zip for `pandora-arm-static`; Wii zip for `wii-powerpc-static`). The workflow publishes a GitHub Release with archives that embed zlib, libpng, SDL3, SDL3_mixer and the bundled DES library statically (WASI archives embed only zlib and DES; the Android APK ships shared `libSDL3.so` + `libmain.so`; the Switch NRO links SDL3 statically from the devkitPro fork; the Dreamcast CDI links SDL3 statically from the GPF Dreamcast fork; the PSP PBP links SDL3 statically from the pspdev packages; the Vita VPK links official SDL3 statically from FetchContent; the Pandora PND links official SDL3 statically against the Ångström sysroot; the Wii DOL links SDL3 statically from the libogc2 fork). OS graphics/audio libraries may still be required at runtime on native builds.
