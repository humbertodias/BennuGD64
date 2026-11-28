[![CI/CD](https://github.com/humbertodias/BennuGD64/actions/workflows/ci.yml/badge.svg)](https://github.com/humbertodias/BennuGD64/actions/workflows/ci.yml)
[![GitHub Pages](https://github.com/humbertodias/BennuGD64/actions/workflows/pages.yml/badge.svg)](https://github.com/humbertodias/BennuGD64/actions/workflows/pages.yml)
[![Ask DeepWiki](https://img.shields.io/badge/Ask-DeepWiki-blue?logo=bookstack)](https://deepwiki.com/humbertodias/BennuGD64)
![GitHub all releases](https://img.shields.io/github/downloads/humbertodias/BennuGD64/total)

# BennuGD64

A fork of [BennuGD](https://www.bennugd.org/) for modern 64-bit architectures.

## Install

The installer builds a **static** `bgdc` (compiler) and `bgdi` (interpreter).

Linux / macOS / Git Bash:

```shell
curl -sL "https://raw.githubusercontent.com/humbertodias/BennuGD64/main/scripts/install.sh" | bash
```

Windows (PowerShell):

```powershell
irm https://raw.githubusercontent.com/humbertodias/BennuGD64/main/scripts/install.ps1 | iex
```

<details>
<summary>Shared modules</summary>

Modules are loaded from `modules/` (`.so` / `.dylib` / `.dll`). Shared builds also install `libbgdrtm`.

Linux / macOS / Git Bash:

```shell
curl -sL "https://raw.githubusercontent.com/humbertodias/BennuGD64/main/scripts/install.sh" | BENNUGD_LINKAGE=shared bash
```

Windows (PowerShell):

```powershell
$env:BENNUGD_LINKAGE = "shared"
irm https://raw.githubusercontent.com/humbertodias/BennuGD64/main/scripts/install.ps1 | iex
```

</details>

<details>
<summary>Libretro core</summary>

```shell
bash scripts/build.sh macos arm64 libretro
bash scripts/build.sh linux libretro
bash scripts/build.sh windows libretro
bash scripts/build.sh android libretro
bash scripts/build.sh switch libretro
```

Load the core in [RetroArch](https://www.retroarch.com/) with `-L` and open a `.dcb` / `.dat`. Keep game assets next to that file. Pass `libretro` after any platform in `scripts/build.sh`. Desktop, Android, iOS, tvOS, and Pandora produce a shared core; most consoles produce a static `bennugd_libretro.a`.

```shell
# macOS (Apple Silicon)
/Applications/RetroArch.app/Contents/MacOS/RetroArch \
  -L dist/macos-arm64-libretro/bennugd_libretro.dylib \
  /path/to/game/main.dcb

# Linux
retroarch -L dist/linux-libretro/bennugd_libretro.so /path/to/game/main.dcb

# Windows
retroarch.exe -L dist/windows-x86_64-libretro/bennugd_libretro.dll C:\path\to\game\main.dcb
```

</details>

## Build

Only Docker is required. See [BUILDING.md](BUILDING.md).

<details>
<summary>Platforms</summary>

```shell
bash scripts/build.sh android
bash scripts/build.sh dreamcast
bash scripts/build.sh ios
bash scripts/build.sh linux
bash scripts/build.sh macos
bash scripts/build.sh macos arm64
bash scripts/build.sh pandora
bash scripts/build.sh zeebo
bash scripts/build.sh ps2
bash scripts/build.sh ps3
bash scripts/build.sh ps4
bash scripts/build.sh ps5
bash scripts/build.sh psp
bash scripts/build.sh switch
bash scripts/build.sh tvos
bash scripts/build.sh vita
bash scripts/build.sh xbox
bash scripts/build.sh xbox360
bash scripts/build.sh wasm
bash scripts/build.sh wii
bash scripts/build.sh windows
bash scripts/build.sh zeebo
```

Add `libretro` after the platform to build the RetroArch core.

</details>

## Getting started

* [BennuGD documentation](https://divhub.github.io/bennugd-website/docs/) — the language.
* [Web player](https://humbertodias.github.io/BennuGD64/) — run `.dcb` games in the browser.
* [Web IDE](https://humbertodias.github.io/BennuGD64/ide/) — edit `.prg`, compile with `bgdc.wasm`, and run in the browser.
* [API reference](https://humbertodias.github.io/BennuGD64/docs/) — C runtime, compiler, and modules.
* [FPG Editor](https://github.com/humbertodias/fpg-editor/) — create and edit FPG files.
* [In-game debugger](DEBUG.md) — `bgdc -g`, `mod_debug`, then step processes in `bgdi`.
