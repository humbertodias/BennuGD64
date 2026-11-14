# Zeebo

Tectoy Zeebo: Qualcomm MSM7201A, ARM1136 at 528 MHz, soft-float, little-endian, 128 MB RAM, VGA 640×480. The system software is Qualcomm BREW 4.0.2, not Linux. Shipped modules are little-endian, which is what Zeebx loads.

`bash scripts/build.sh zeebo` produces `dist/zeebo-arm-static/bgdi.elf` (and `bennugd_libretro.a` with `libretro`). The ELF is ARMv6, ARM state, soft-float, little-endian, linked against newlib. `hello.dcb` is copied as `main.dcb`.

`bgdi.elf` also exports `AEEMod_Load`. The image packs it with `elf2mod` and lays the SD card tree out as `mif/bgdi.mif` and `mod/bgdi/bgdi.mod`, with `main.dcb` in the module folder. `platforms/zeebo/bgdi.brx` is the MIF source (applet name BennuGD64, class `0x0100B6D1`). Copy this console's test signature to `mod/bgdi/bgdi.sig`; Qualcomm issues that file for the handset IMEI.

There is no hardware FPU. Floating point goes through libgcc soft-float helpers.
