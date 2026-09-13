# Original Xbox (nxdk)

This port targets softmodded Original Xbox consoles through the open
[nxdk](https://github.com/XboxDev/nxdk) SDK. It produces a `default.xbe`; it
does not use the Microsoft XDK or lantus SDLx.

The `/Users/.../xSorRv5_Update1` release is useful as a **data-layout and
controls** reference (`D:\`, `SorR.dat`, pad mapping). Its `default.xbe`,
Direct3D 8 and XDK libraries are not reused.

## Build

From the repository root:

```sh
bash scripts/build.sh xbox
```

The package is written to `dist/xbox-i386-static/`:

- `default.xbe` — softmod / HDD title
- `bennugd64.iso` — XISO for [xemu](https://xemu.app/) (`extract-xiso -c`)
- sample `main.dcb` (from the host demo)

The toolchain image is based on `xboxdev/nxdk` (`linux/amd64`) and contains no
Microsoft SDK files.

## Install

### xemu

Open `dist/xbox-i386-static/bennugd64.iso` as the disc image. The sample ISO
includes `default.xbe` and the demo `main.dcb` at the root.

To test Streets of Rage Remake (or another game), build a custom XISO whose
root matches the HDD layout below (at least `default.xbe` + `SorR.dat` /
`main.dcb` and assets), for example:

```sh
extract-xiso -c ./my-game-folder ./sorr.iso
```

### Softmodded console / xemu HDD

1. Softmod an Original Xbox and install a dashboard that can launch XBEs,
   or use the [xemu dashboard](https://github.com/xemu-project/xemu-dashboard)
   HDD image.
2. Copy `default.xbe` and `main.dcb` (or `SorR.dat` + assets) into the same
   folder on the HDD, e.g. `E:\bennugd64\` (via dashboard FTP — see below).
3. Launch that `default.xbe` from the dashboard. The runtime remaps `D:` to
   the folder that contains the XBE.

#### Putting files on `xbox_hdd.qcow2` (xemu)

Easiest on macOS/Windows/Linux: use the dashboard FTP while xemu is running
(with the dashboard, not our ISO, as the disc — or boot the dashboard XBE).

1. In xemu: **Machine → Network** → Attached to **NAT**, forward host `2121`
   → guest `21` (TCP), then Enable.
2. Boot the xemu dashboard from the HDD (no need to load our ISO for this).
3. Connect an FTP client to `127.0.0.1:2121` (user/pass usually `xbox` /
   `xbox`). For FileZilla details see
   [xemu FTP docs](https://xemu.app/docs/ftp/).
4. Upload into e.g. `E:/bennugd64/`:
   - `default.xbe`
   - `main.dcb` (or full game data)
5. From the dashboard, launch `E:\bennugd64\default.xbe`.

Alternatives: [FATXplorer](https://fatxplorer.eaton-essers.com/) (Windows) on
the qcow/raw image, or `qemu-nbd` + [fatxfs](https://github.com/mborgerson/fatx)
on Linux.

For Streets of Rage Remake, a layout matching xSorR is:

```text
D:\default.xbe
D:\SorR.dat
D:\mod\
D:\palettes\
D:\savegame\
```

or:

```text
D:\default.xbe
D:\bennugd64\SorR.dat
D:\bennugd64\mod\
D:\bennugd64\palettes\
```

## Controls

- D-pad / left stick: arrows
- A: `A`, `C`, Control
- B: `D`, `V`, Alt
- X: `S`, `X`, Space
- Y / Black: `B`
- Start: Enter
- Back: Escape

Native joystick functions expose up to four Xbox controllers, two sticks,
triggers and one D-pad hat.
