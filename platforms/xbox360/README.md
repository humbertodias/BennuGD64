# Xbox 360 (libXenon)

This port targets homebrew-enabled Xbox 360 consoles through the open
Free60/libXenon stack. It produces an ELF for XeLL Reloaded; it does not use
Microsoft XDK headers or generate an XEX.

## Build

From the repository root:

```sh
bash scripts/build.sh xbox360
```

The package is written to `dist/xbox360-powerpc-static/`. The toolchain image
is based on `free60/libxenon` and contains no Microsoft SDK files.

## Install

1. Format a USB drive as FAT32.
2. Copy `dist/xbox360-powerpc-static/xenon.elf` to the USB root.
3. Create `bennugd64/` on that drive.
4. Copy `main.dcb` and every game asset into `bennugd64/`.
5. Boot the USB `xenon.elf` with XeLL Reloaded.

For Streets of Rage Remake, the resulting layout is:

```text
/xenon.elf
/bennugd64/SorR.dat
/bennugd64/mod/
/bennugd64/palettes/
/bennugd64/...
```

The bootstrap accepts either `main.dcb` or `SorR.dat`. The original Xbox
release is useful as a data-layout reference, but its `default.xbe`, Direct3D
8/SDLx code and Xbox SDK libraries are not compatible with Xbox 360.

The runtime searches `uda:`, `udb:`, `udc:` and the internal `sda:` FAT
device. The first mounted FAT device becomes the current working directory.

## Controls

- D-pad / left stick: arrows
- A: `A`, `C`, Control
- B: `D`, `V`, Alt
- X: `S`, `X`, Space
- Y: `B`
- Start: Enter
- Back: Escape

Native joystick functions expose up to four Xbox 360 controllers, two sticks,
both triggers and one D-pad hat.
