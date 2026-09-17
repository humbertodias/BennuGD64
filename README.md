[![CI/CD](https://github.com/humbertodias/BennuGD64/actions/workflows/ci.yml/badge.svg)](https://github.com/humbertodias/BennuGD64/actions/workflows/ci.yml)
[![GitHub Pages](https://github.com/humbertodias/BennuGD64/actions/workflows/pages.yml/badge.svg)](https://github.com/humbertodias/BennuGD64/actions/workflows/pages.yml)
[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/humbertodias/BennuGD64)
![GitHub all releases](https://img.shields.io/github/downloads/humbertodias/BennuGD64/total)

# BennuGD64

A fork of [BennuGD](https://www.bennugd.org/) adapted to run on modern 64-bit architectures.

## Install

The installer defaults to a **static** build (modules linked into `bgdi`).

Linux / macOS / Git Bash:

```shell
curl -sL "https://raw.githubusercontent.com/humbertodias/BennuGD64/main/scripts/install.sh" | bash
```

Shared modules (`.so` / `.dylib` under `modules/`):

```shell
curl -sL "https://raw.githubusercontent.com/humbertodias/BennuGD64/main/scripts/install.sh" | BENNUGD_LINKAGE=shared bash
```

Windows (PowerShell):

```powershell
irm https://raw.githubusercontent.com/humbertodias/BennuGD64/main/scripts/install.ps1 | iex
```

Shared modules (`.dll` under `modules/`):

```powershell
$env:BENNUGD_LINKAGE = "shared"
irm https://raw.githubusercontent.com/humbertodias/BennuGD64/main/scripts/install.ps1 | iex
```

It installs `bgdc` (compiler) and `bgdi` (interpreter).
Shared builds also include `libbgdrtm` and load plugins from `modules/`.

Libretro core (`bennugd_libretro`):

```shell
bash scripts/build.sh macos arm64 libretro
bash scripts/build.sh linux libretro
bash scripts/build.sh windows libretro
bash scripts/build.sh android libretro
bash scripts/build.sh switch libretro
```

Load the core in [RetroArch](https://www.retroarch.com/) with `-L` and open a `.dcb` / `.dat`. Put game assets next to that file.

macOS (Apple Silicon):

```shell
/Applications/RetroArch.app/Contents/MacOS/RetroArch \
  -L dist/macos-arm64-libretro/bennugd_libretro.dylib \
  /path/to/game/main.dcb
```

Linux:

```shell
retroarch -L dist/linux-libretro/bennugd_libretro.so /path/to/game/main.dcb
```

Windows (from Git Bash or PowerShell, after a MinGW build):

```shell
retroarch.exe -L dist/windows-x86_64-libretro/bennugd_libretro.dll C:\path\to\game\main.dcb
```

Pass `libretro` after any other platform in `scripts/build.sh` (same Docker image as the standalone build). Desktop, Android, iOS, tvOS, and Pandora produce a shared core; most consoles produce a static `bennugd_libretro.a` for linking into RetroArch.

## Build

Only Docker is required; no local compiler or CMake installation is needed. See [BUILDING.md](BUILDING.md)

```shell
bash scripts/build.sh android
bash scripts/build.sh dreamcast
bash scripts/build.sh ios
bash scripts/build.sh linux
bash scripts/build.sh linux libretro
bash scripts/build.sh macos
bash scripts/build.sh macos arm64
bash scripts/build.sh macos arm64 libretro
bash scripts/build.sh pandora
bash scripts/build.sh ps2
bash scripts/build.sh ps3
bash scripts/build.sh ps4
bash scripts/build.sh psp
bash scripts/build.sh switch
bash scripts/build.sh tvos
bash scripts/build.sh vita
bash scripts/build.sh wasm
bash scripts/build.sh wii
bash scripts/build.sh windows
bash scripts/build.sh windows libretro
```

## Getting started

Learn the Bennu language with the [BennuGD documentation](https://divhub.github.io/bennugd-website/docs/).

* [Web player](https://humbertodias.github.io/BennuGD64/) - run `.dcb` games directly in the browser using WebAssembly.
* [Web IDE](https://humbertodias.github.io/BennuGD64/ide/) - edit `.prg` files, compile with `bgdc.wasm`, and run them in the browser.
* [API reference](https://humbertodias.github.io/BennuGD64/docs/) - Doxygen documentation for the C runtime, compiler, and modules.
* [FPG Editor](https://github.com/humbertodias/fpg-editor/) - create and edit FPG files easily.

Enjoy!
