# Zeebo

Tectoy Zeebo: Qualcomm MSM7201A, ARM1136 at 528 MHz, soft-float, big-endian, 128 MB RAM, VGA 640×480. The system software is Qualcomm BREW 4.0.2, not Linux.

`bash scripts/build.sh zeebo` produces `dist/zeebo-arm-static/bgdi.elf` (and `bennugd_libretro.a` with `libretro`). The ELF is ARMv6, ARM state, soft-float, big-endian, linked against newlib. `hello.dcb` is copied as `main.dcb`.

`bgdi.elf` also exports `AEEMod_Load`, the BREW module entry. Qualcomm's `elf2mod` (BREW toolset) is not redistributable, so this image stops at the ELF. Pack that ELF with `elf2mod` to get a `.mod` the console or a BREW loader can start.

There is no hardware FPU. Floating point goes through libgcc soft-float helpers.
