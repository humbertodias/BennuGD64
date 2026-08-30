[![CI/CD](https://github.com/humbertodias/BennuGD64/actions/workflows/ci.yml/badge.svg)](https://github.com/humbertodias/BennuGD64/actions/workflows/ci.yml)
[![GitHub Pages](https://github.com/humbertodias/BennuGD64/actions/workflows/pages.yml/badge.svg)](https://github.com/humbertodias/BennuGD64/actions/workflows/pages.yml)
[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/humbertodias/BennuGD64)
![GitHub all releases](https://img.shields.io/github/downloads/humbertodias/BennuGD64/total)

# BennuGD64

A fork of [BennuGD](https://www.bennugd.org/) adapted to run on modern 64-bit architectures.

## Web & docs

- [Web player](https://humbertodias.github.io/BennuGD64/) - run `.dcb` games in the browser (WebAssembly). Drop a bytecode file plus assets, or pick a bundled demo.
- [Web IDE](https://humbertodias.github.io/BennuGD64/ide/) - edit `.prg` in Monaco, compile with `bgdc.wasm`, detect the DCB version, run in `bgdi`.
- [API reference](https://humbertodias.github.io/BennuGD64/docs/) - Doxygen docs for the C runtime, compiler, and modules (published from `main` via GitHub Pages).

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

It will install two tools: `bgdc` *compiler* and `bgdi` *interpreter*. Shared builds also place `libbgdrtm` next to the binaries and load plugins from `modules/`.

## Build

Only Docker is required for Linux, Windows, macOS (osxcross), the web player, Android, Nintendo Switch, Sega Dreamcast, PlayStation Portable, PlayStation Vita, PlayStation 2, OpenPandora, and Nintendo Wii (no local compiler or CMake):

```shell
bash scripts/build.sh windows
bash scripts/build.sh linux
bash scripts/build.sh macos
bash scripts/build.sh macos arm64
bash scripts/build.sh wasm
bash scripts/build.sh android
bash scripts/build.sh switch
bash scripts/build.sh dreamcast
bash scripts/build.sh psp
bash scripts/build.sh vita
bash scripts/build.sh ps2
bash scripts/build.sh pandora
bash scripts/build.sh wii
```

See [BUILDING.md](BUILDING.md) for Windows, wasm, WASI, Android, Switch, Dreamcast, PSP, Vita, PS2, Pandora, Wii, macOS (Docker osxcross and native), and native CMake.


## Getting started

Learn the Bennu language with the [BennuGD documentation](https://divhub.github.io/bennugd-website/docs/). For engine internals, see the [API reference](https://humbertodias.github.io/BennuGD64/docs/).

Use the [FPG Editor](https://github.com/humbertodias/fpg-editor/) to make it easier.

Enjoy!
