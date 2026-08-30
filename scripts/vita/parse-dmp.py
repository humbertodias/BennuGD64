#!/usr/bin/env python3
"""Parse a PlayStation Vita .psp2dmp crash dump (gzip ELF core).

  python3 scripts/vita/parse-dmp.py dump.psp2dmp [bgdi.elf]

The dump is gzip-compressed ELF. This prints threads, stop reason, PC/LR, and
optional symbols from an unstripped bgdi.elf (not eboot.bin / .velf).
For a full disassembly around PC, use https://github.com/xyzz/vita-parse-core
with vitasdk's arm-vita-eabi-addr2line.
"""
from __future__ import annotations

import gzip
import struct
import sys
from pathlib import Path

STOP = {
    0: "No reason",
    0x30002: "Undefined instruction",
    0x30003: "Prefetch abort",
    0x30004: "Data abort",
    0x60080: "Division by zero",
}


def u32(d: bytes, o: int) -> int:
    return struct.unpack_from("<I", d, o)[0]


def u16(d: bytes, o: int) -> int:
    return struct.unpack_from("<H", d, o)[0]


def cstr(d: bytes, o: int, n: int = 32) -> str:
    return d[o : o + n].split(b"\x00", 1)[0].decode("ascii", "replace")


def load_core(path: Path) -> bytes:
    raw = path.read_bytes()
    if raw[:2] == b"\x1f\x8b":
        return gzip.decompress(raw)
    return raw


def notes(data: bytes) -> dict[str, bytes]:
    e_phoff = u32(data, 28)
    e_phentsize = struct.unpack_from("<H", data, 42)[0]
    e_phnum = struct.unpack_from("<H", data, 44)[0]
    out: dict[str, bytes] = {}
    for i in range(e_phnum):
        off = e_phoff + i * e_phentsize
        p_type, p_offset, _v, _p, p_filesz = struct.unpack_from("<IIIII", data, off)
        if p_type != 4:
            continue
        note = data[p_offset : p_offset + p_filesz]
        namesz, descsz, _nt = struct.unpack_from("<III", note, 0)
        name = note[12 : 12 + namesz].split(b"\x00", 1)[0].decode("ascii", "replace")
        pos = (12 + namesz + 3) & ~3
        out[name] = note[pos : pos + descsz]
    return out


def elf_loads(data: bytes) -> list[tuple[int, int]]:
    """(link_vaddr, filesz) of PT_LOAD RX then RW."""
    e_phoff = u32(data, 28)
    e_phentsize = struct.unpack_from("<H", data, 42)[0]
    e_phnum = struct.unpack_from("<H", data, 44)[0]
    loads = []
    for i in range(e_phnum):
        off = e_phoff + i * e_phentsize
        p_type, p_offset, p_vaddr, _p, p_filesz, p_memsz, p_flags, _a = struct.unpack_from(
            "<IIIIIIII", data, off
        )
        if p_type == 1:
            loads.append((p_vaddr, p_memsz, p_flags))
    return loads


def parse_modules(mod: bytes) -> list[tuple[str, int, int]]:
    """name, seg1 start, seg1 size (RX)."""
    n = u32(mod, 4)
    off = 8
    mods = []
    for _ in range(n):
        name = cstr(mod, off + 0x24, 32)
        nseg = u32(mod, off + 0x4C)
        off += 0x50
        segs = []
        for _s in range(nseg):
            _attr, start, size, _align = struct.unpack_from("<IIII", mod, off + 4)
            segs.append((start, size))
            off += 0x14
        off += 0x10
        if segs:
            mods.append((name, segs[0][0], segs[0][1]))
    return mods


def load_symbols(elf_path: Path) -> list[tuple[int, int, str]]:
    data = elf_path.read_bytes()
    e_shoff = u32(data, 32)
    e_shentsize = struct.unpack_from("<H", data, 46)[0]
    e_shnum = struct.unpack_from("<H", data, 48)[0]
    e_shstrndx = struct.unpack_from("<H", data, 50)[0]

    def shdr(i: int) -> tuple:
        off = e_shoff + i * e_shentsize
        return struct.unpack_from("<IIIIIIIIII", data, off)

    shstr = shdr(e_shstrndx)
    shstrtab = data[shstr[4] : shstr[4] + shstr[5]]

    def sname(nidx: int) -> str:
        return shstrtab[nidx:].split(b"\x00", 1)[0].decode()

    syms: list[tuple[int, int, str]] = []
    for i in range(e_shnum):
        n, t, f, addr, off, sz, link, info, al, es = shdr(i)
        if sname(n) != ".symtab":
            continue
        strs = shdr(link)
        strtab = data[strs[4] : strs[4] + strs[5]]
        entsz = es or 16
        for j in range(0, sz, entsz):
            st_name, st_value, st_size, st_info, _o, _sh = struct.unpack(
                "<IIIBBH", data[off + j : off + j + entsz]
            )
            nm = strtab[st_name:].split(b"\x00", 1)[0].decode("ascii", "replace")
            if nm and st_size:
                syms.append((st_value, st_size, nm))
    syms.sort()
    return syms


def lookup(syms: list[tuple[int, int, str]], addr: int) -> str:
    for v, sz, nm in reversed(syms):
        if v <= addr < v + sz:
            return f"{nm}+0x{addr - v:x}"
        if v <= addr:
            return f"{nm}+0x{addr - v:x} (size 0?)"
    return "?"


def tty_excerpt(desc: bytes) -> str:
    text = "".join(chr(b) if 32 <= b < 127 else "\n" for b in desc)
    lines = [ln.strip() for ln in text.splitlines() if ln.strip()]
    keep = [
        ln
        for ln in lines
        if "Library not found" in ln
        or "ERROR:" in ln
        or "[coredump]" in ln
        or "Runtime error" in ln
    ]
    return "\n".join(keep[:20])


def main() -> int:
    if len(sys.argv) < 2:
        print(__doc__.strip(), file=sys.stderr)
        return 2
    dmp = Path(sys.argv[1])
    elf_path = Path(sys.argv[2]) if len(sys.argv) > 2 else None
    data = load_core(dmp)
    if data[:4] != b"\x7fELF":
        print("not an ELF core (after gzip)", file=sys.stderr)
        return 1
    n = notes(data)
    print(f"file: {dmp} ({len(data)} bytes uncompressed)")
    if b"PROCESS_INFO" in n or "PROCESS_INFO" in n:
        proc = n["PROCESS_INFO"]
        print("process:", cstr(proc, 0x10, 32) if len(proc) > 48 else proc[16:48])
        # path often near the end of the 148-byte blob
        print("info:", "".join(chr(b) if 32 <= b < 127 else " " for b in proc))

    mods = parse_modules(n["MODULE_INFO"]) if "MODULE_INFO" in n else []
    bgdi = next((m for m in mods if "bgdi" in m[0] or m[0].endswith(".elf")), None)
    text_slide = 0
    if bgdi and elf_path and elf_path.is_file():
        elff = elf_path.read_bytes()
        loads = elf_loads(elff)
        rx = next((v for v, _m, fl in loads if fl & 5 == 5 or fl & 1), None)
        if rx is None and loads:
            rx = loads[0][0]
        if rx is not None:
            text_slide = bgdi[1] - rx
            print(f"bgdi.elf runtime RX 0x{bgdi[1]:08x} linked 0x{rx:08x} slide 0x{text_slide:x}")
    elif bgdi:
        print(f"bgdi runtime RX 0x{bgdi[1]:08x} (pass bgdi.elf to resolve symbols)")

    syms = load_symbols(elf_path) if elf_path and elf_path.is_file() else []

    th = n["THREAD_INFO"]
    num = u32(th, 4)
    off = 8
    print("\n=== THREADS ===")
    crashed = []
    threads = []
    for i in range(num):
        sz = u32(th, off)
        uid = u32(th, off + 4)
        name = cstr(th, off + 8, 32)
        status = u16(th, off + 0x30)
        stop = u32(th, off + 0x74)
        pc = u32(th, off + 0x9C)
        threads.append((uid, name, stop, pc))
        if stop:
            crashed.append(i)
        extra = STOP.get(stop, "")
        print(f"  {name!r} uid=0x{uid:x} stop=0x{stop:x} {extra} pc=0x{pc:08x}")
        off += sz

    regs = n["THREAD_REG_INFO"]
    numr = u32(regs, 4)
    off = 8
    by_tid = {}
    for _ in range(numr):
        sz = u32(regs, off)
        tid = u32(regs, off + 4)
        gpr = [u32(regs, off + 8 + 4 * x) for x in range(16)]
        by_tid[tid] = gpr
        off += sz

    for i in crashed or [0]:
        uid, name, stop, pc = threads[i]
        gpr = by_tid.get(uid)
        if not gpr:
            continue
        print(f"\n=== CRASH {name!r} ({STOP.get(stop, hex(stop))}) ===")
        print(f"  R0-R3  {[hex(x) for x in gpr[:4]]}")
        print(f"  SP={gpr[13]:08x} LR={gpr[14]:08x} PC={gpr[15]:08x}")
        if text_slide and syms:
            for lab, addr in (("PC", gpr[15]), ("LR", gpr[14] & ~1)):
                linked = addr - text_slide
                print(f"  {lab} linked 0x{linked:08x}  {lookup(syms, linked)}")

    if "TTY_INFO2" in n:
        excerpt = tty_excerpt(n["TTY_INFO2"])
        if excerpt:
            print("\n=== TTY ===")
            print(excerpt)
    return 0


if __name__ == "__main__":
    sys.exit(main())
