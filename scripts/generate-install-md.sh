#!/usr/bin/env bash
set -euo pipefail

artifact="${1:?artifact name is required}"
output_dir="${2:?output directory is required}"

case "${artifact}" in
  *-linux-*)
    platform="Linux"
    install='Run `./bgdi main.dcb`. Use `./bgdc source.prg` to compile BennuGD programs.'
    data='Place the DCB and its assets together in a writable game directory.'
    ;;
  *-windows-*)
    platform="Windows"
    install='Run `bgdi.exe main.dcb`. Use `bgdc.exe source.prg` to compile BennuGD programs.'
    data='Place the DCB and its assets together in a writable game directory.'
    ;;
  *-macos-*)
    platform="macOS"
    install='Run `./bgdi main.dcb`. Use `./bgdc source.prg` to compile BennuGD programs.'
    data='Place the DCB and its assets together in a writable game directory.'
    ;;
  *-web-wasm32-*)
    platform="WebAssembly browser"
    install='Serve this directory over HTTP and open `index.html`; loading it directly with a `file://` URL is not supported.'
    data='The archive includes the browser interpreter and Web IDE under `ide/`, with compiler and sample assets.'
    ;;
  *-wasi-wasm32-*)
    platform="WASI"
    install='Run `wasmtime --dir=. ./bgdc.wasm -- -o out.dcb in.prg`.'
    data='Grant additional directories to the WASI runtime when source files or output live outside this folder.'
    ;;
  *-android-arm64-*)
    platform="Android arm64"
    install='Install `bennugd64.apk` on an arm64-v8a device running Android API 28 or newer.'
    data='The archive also contains `libmain.so` and `libSDL3.so` for integration and diagnostics.'
    ;;
  *-switch-aarch64-*)
    platform="Nintendo Switch"
    install='Copy `bennugd64.nro` to `sdmc:/switch/bennugd64/` or send it with `nxlink`.'
    data='Place `main.dcb` and game assets beside the NRO.'
    ;;
  *-dreamcast-sh4-*)
    platform="Sega Dreamcast"
    install='Boot or burn `bennugd64.cdi`; developers can send `bgdi.elf` with `dc-tool`.'
    data='Rebuild the CDI when replacing the bundled DCB or assets.'
    ;;
  *-psp-mips-*)
    platform="PlayStation Portable"
    install='Copy the extracted BennuGD64 folder containing `EBOOT.PBP` to `ms0:/PSP/GAME/bennugd64/`.'
    data='Place `main.dcb` and game assets in the same folder.'
    ;;
  *-vita-arm-*)
    platform="PlayStation Vita"
    install='Install `bennugd64.vpk` on a homebrew-enabled Vita.'
    data='Place additional game data in `ux0:/data/bennugd64/`.'
    ;;
  *-tvos-simulator-arm64-*)
    platform="tvOS Simulator"
    install='On macOS, run `./sim-install.sh`. If needed, restore execute permission with `chmod +x sim-install.sh bgdi.app/bgdi`.'
    data='Place `main.dcb` at the `bgdi.app` root before installation.'
    ;;
  *-tvos-arm64-*)
    platform="Apple tvOS"
    install='Sign the unsigned `bgdi.app` with your Apple development identity, then deploy it to Apple TV.'
    data='Place `main.dcb` at the app root, or add it under `platforms/tvos/contents/` before rebuilding.'
    ;;
  *-ios-simulator-arm64-*)
    platform="iOS Simulator"
    install='On macOS, run `./sim-install.sh`. If needed, restore execute permission with `chmod +x sim-install.sh bgdi.app/bgdi`.'
    data='Place `main.dcb` at the `bgdi.app` root before installation.'
    ;;
  *-ios-arm64-*)
    platform="Apple iOS"
    install='Sign the unsigned `bgdi.app` with your Apple development identity, then deploy it to an iPhone or iPad.'
    data='Place `main.dcb` at the app root or in the app Documents directory.'
    ;;
  *-ps2-mips-*)
    platform="PlayStation 2"
    install='Open `bennugd64.iso` in PCSX2, boot it on a homebrew-enabled console, or open `bgdi.elf` through PCSX2 HostFS.'
    data='External game data can be loaded from `mass:/`; HostFS users can keep assets beside the ELF.'
    ;;
  *-ps3-ppu-*)
    platform="PlayStation 3"
    install='Install `bennugd64.pkg` on a CFW or homebrew-enabled PS3.'
    data='Place additional game data in `/dev_usb000/bennugd64/` or bundle it in `USRDIR`.'
    ;;
  *-ps4-x86_64-*)
    platform="PlayStation 4"
    install='Install `bennugd64.pkg` on a jailbroken PS4.'
    data='Place `main.dcb` and game assets in `/mnt/usb0/bennugd64/` or `/data/bennugd64/`.'
    ;;
  *-xbox360-powerpc-*)
    platform="Xbox 360"
    install='Copy `xenon.elf` to the root of a FAT32 USB drive and boot it with XeLL Reloaded. This is a libXenon homebrew binary, not an XEX.'
    data='Place `main.dcb` and all game assets in `uda:/bennugd64/`. Additional USB devices are searched as `udb:` and `udc:`.'
    ;;
  *-xbox-i386-*)
    platform="Original Xbox"
    install='Open `bennugd64.iso` in [xemu](https://xemu.app/), or copy `default.xbe` to a softmodded Original Xbox game folder and launch it from a dashboard such as XBMC4Gamers or UnleashX.'
    data='Place `main.dcb` (or `SorR.dat`) and assets in `D:\bennugd64\` or beside the XBE on `D:\`. The sample ISO already embeds the demo `main.dcb` at the disc root.'
    ;;
  *-pandora-arm-*)
    platform="OpenPandora"
    install='Copy `bennugd64.pnd` to an application directory on the Pandora SD card, or run `./bgdi` directly.'
    data='For direct execution, place `main.dcb` and its assets beside `bgdi`.'
    ;;
  *-wii-powerpc-*)
    platform="Nintendo Wii"
    install='Copy the included `apps/bennugd64/` folder to the SD card and launch it from the Homebrew Channel.'
    data='Place `main.dcb` and game assets in `sd:/apps/bennugd64/`.'
    ;;
  *)
    echo "Unsupported release artifact: ${artifact}" >&2
    exit 1
    ;;
esac

if [[ "${artifact}" == *-shared ]]; then
  linkage='This is a shared build. Keep the `modules/` directory beside the interpreter.'
else
  linkage='This is a static build. BennuGD modules are linked into the interpreter.'
fi

cat > "${output_dir}/INSTALL.md" <<EOF
# Installing BennuGD64 on ${platform}

## Installation

${install}

## Game data

${data}

## Build layout

${linkage}

Third-party libraries are linked statically where supported. Platform graphics,
audio, system libraries, homebrew loaders, signing tools, or runtimes may still
be required.
EOF
