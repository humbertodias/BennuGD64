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

## Build

Only Docker is required; no local compiler or CMake installation is needed. See [BUILDING.md](BUILDING.md)

```shell
bash scripts/build.sh android
bash scripts/build.sh dreamcast
bash scripts/build.sh ios
bash scripts/build.sh linux
bash scripts/build.sh macos
bash scripts/build.sh macos arm64
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
```

## Getting started

Learn the Bennu language with the [BennuGD documentation](https://divhub.github.io/bennugd-website/docs/).

* [Web player](https://humbertodias.github.io/BennuGD64/) - run `.dcb` games directly in the browser using WebAssembly.
* [Web IDE](https://humbertodias.github.io/BennuGD64/ide/) - edit `.prg` files, compile with `bgdc.wasm`, and run them in the browser.
* [API reference](https://humbertodias.github.io/BennuGD64/docs/) - Doxygen documentation for the C runtime, compiler, and modules.
* [FPG Editor](https://github.com/humbertodias/fpg-editor/) - create and edit FPG files easily.

Enjoy!
