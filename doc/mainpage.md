# BennuGD64

BennuGD64 is a fork of [BennuGD](https://www.bennugd.org/) adapted for modern 64-bit
architectures and multiple homebrew / sandboxed targets.

This reference is generated automatically from the C sources with [Doxygen](https://www.doxygen.nl/).

## Components

| Binary | Role |
|--------|------|
| `bgdc` | Compiler — turns `.prg` sources into `.dcb` bytecode |
| `bgdi` | Interpreter — loads `.dcb` and runs the game |

## Layout

- **core/** — runtime (`bgdrtm`), interpreter (`bgdi`), compiler (`bgdc`), shared headers
- **modules/** — engine libraries (`lib*`) and Bennu language modules (`mod_*`)

Port-specific code lives in `*_psp.c`, `*_vita.c`, `*_tvos.c`, `*_ios.c`, `*_switch.c`, `*_win32.c`, and similar units that
CMake adds only for the matching target.

## User documentation

Game scripting and tutorials: [BennuGD documentation](https://divhub.github.io/bennugd-website/docs/)

Build instructions: [BUILDING.md](https://github.com/humbertodias/BennuGD64/blob/main/BUILDING.md)
