#!/usr/bin/env python3
"""Pad a Wii DOL so every section size is 32-byte aligned.

Dolphin's DolReader rejects DOLs whose text/data sizes are not multiples of 32
(Failed to init core). elf2dol copies ELF section sizes as-is.
"""
from __future__ import annotations

import struct
import sys
from pathlib import Path

NUM_TEXT = 7
NUM_DATA = 11
HEADER_SIZE = 0x100


def align32(n: int) -> int:
    return (n + 31) & ~31


def pad_dol(src: Path, dst: Path) -> None:
    data = src.read_bytes()
    if len(data) < HEADER_SIZE:
        raise SystemExit(f"{src}: too small to be a DOL")

    def u32s(offset: int, count: int) -> list[int]:
        return list(struct.unpack(f">{count}I", data[offset : offset + 4 * count]))

    text_off = u32s(0x00, NUM_TEXT)
    data_off = u32s(0x1C, NUM_DATA)
    text_addr = u32s(0x48, NUM_TEXT)
    data_addr = u32s(0x64, NUM_DATA)
    text_size = u32s(0x90, NUM_TEXT)
    data_size = u32s(0xAC, NUM_DATA)
    bss_addr, bss_size, entry = struct.unpack(">3I", data[0xD8:0xE4])

    sections: list[tuple[list[int], list[int], list[int], int]] = []
    for i in range(NUM_TEXT):
        if text_size[i]:
            sections.append((text_off, text_addr, text_size, i))
    for i in range(NUM_DATA):
        if data_size[i]:
            sections.append((data_off, data_addr, data_size, i))

    out = bytearray(HEADER_SIZE)
    cursor = HEADER_SIZE
    for offs, _addrs, sizes, i in sections:
        chunk = data[offs[i] : offs[i] + sizes[i]]
        padded = align32(len(chunk))
        offs[i] = cursor
        sizes[i] = padded
        out.extend(chunk)
        out.extend(b"\x00" * (padded - len(chunk)))
        cursor += padded

    def pack(offset: int, values: list[int]) -> None:
        out[offset : offset + 4 * len(values)] = struct.pack(f">{len(values)}I", *values)

    pack(0x00, text_off)
    pack(0x1C, data_off)
    pack(0x48, text_addr)
    pack(0x64, data_addr)
    pack(0x90, text_size)
    pack(0xAC, data_size)
    out[0xD8:0xE4] = struct.pack(">3I", bss_addr, bss_size, entry)

    dst.write_bytes(out)


def main() -> None:
    if len(sys.argv) not in (2, 3):
        raise SystemExit("usage: pad-wii-dol.py <boot.dol> [out.dol]")
    src = Path(sys.argv[1])
    dst = Path(sys.argv[2]) if len(sys.argv) == 3 else src
    pad_dol(src, dst)


if __name__ == "__main__":
    main()
