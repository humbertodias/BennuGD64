[![CI/CD](https://github.com/humbertodias/BennuGD64/actions/workflows/ci.yml/badge.svg)](https://github.com/humbertodias/BennuGD64/actions/workflows/ci.yml)
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

It will install two tools: `bgdc` *compiler* and `bgdi` *interpreter*. Shared builds also place `libbgdrtm` next to the binaries and load plugins from `modules/`.

## Build

Only Docker is required for Linux, Windows, the web player, Android, and Nintendo Switch (no local compiler or CMake):

```shell
bash scripts/docker-build.sh linux
bash scripts/docker-build.sh wasm
bash scripts/docker-build.sh android
bash scripts/docker-build.sh switch
```

See [BUILDING.md](BUILDING.md) for Windows, wasm, WASI, Android, Switch, macOS, and native CMake.


## Getting starting

Learn using the [BennuGD documentation](https://divhub.github.io/bennugd-website/docs/)

Use the [FPG Editor](https://github.com/humbertodias/fpg-editor/) to make it easier.

Enjoy!
