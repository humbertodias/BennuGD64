#!/usr/bin/env python3
"""Drop relocations Qualcomm elf2mod refuses.

elf2mod only places a symbol that sits strictly inside a PT_LOAD. Unwind
tables add R_ARM_NONE, R_ARM_PREL31, and relocations against .ARM.exidx
(__exidx_start / __exidx_end), which the loader reports as outside a segment.
The module does not use those tables.
"""

import struct
import sys

R_ARM_NONE = 0
R_ARM_PREL31 = 42
PT_LOAD = 1
SHT_REL = 9
SHT_SYMTAB = 2


def main():
    if len(sys.argv) not in (2, 3):
        sys.exit(f"usage: {sys.argv[0]} input.elf [output.elf]")
    src = sys.argv[1]
    dst = sys.argv[2] if len(sys.argv) == 3 else src
    data = bytearray(open(src, "rb").read())
    if data[:4] != b"\x7fELF" or data[4] != 1 or data[5] not in (1, 2):
        sys.exit("expected an ELF32")
    endian = "<" if data[5] == 1 else ">"

    e_phoff = struct.unpack_from(endian + "I", data, 28)[0]
    e_shoff = struct.unpack_from(endian + "I", data, 32)[0]
    e_phentsize = struct.unpack_from(endian + "H", data, 42)[0]
    e_phnum = struct.unpack_from(endian + "H", data, 44)[0]
    e_shentsize = struct.unpack_from(endian + "H", data, 46)[0]
    e_shnum = struct.unpack_from(endian + "H", data, 48)[0]

    non_load = []
    for i in range(e_phnum):
        off = e_phoff + i * e_phentsize
        p_type, _, p_vaddr, _, p_filesz, _, _, _ = struct.unpack_from(endian + "IIIIIIII", data, off)
        if p_type != PT_LOAD and p_type != 0 and p_filesz:
            non_load.append((p_vaddr, p_vaddr + p_filesz))

    def shdr(i):
        return struct.unpack_from(endian + "IIIIIIIIII", data, e_shoff + i * e_shentsize)

    exidx_sections = set()
    symtab = None
    for i in range(e_shnum):
        sh_addr, sh_offset, sh_size, sh_type = shdr(i)[3], shdr(i)[4], shdr(i)[5], shdr(i)[1]
        if sh_type == SHT_SYMTAB:
            symtab = (sh_offset, sh_size)
        if any(lo <= sh_addr < hi for lo, hi in non_load):
            exidx_sections.add(i)

    if symtab is None:
        sys.exit("ELF has no symbol table")
    sym_off, _sym_size = symtab

    def symbol_shndx(index):
        return struct.unpack_from(endian + "H", data, sym_off + index * 16 + 14)[0]

    dropped = 0
    for i in range(e_shnum):
        if shdr(i)[1] != SHT_REL:
            continue
        off = e_shoff + i * e_shentsize
        sh_offset = shdr(i)[4]
        sh_size = shdr(i)[5]
        kept = bytearray()
        for n in range(sh_size // 8):
            ent = data[sh_offset + n * 8 : sh_offset + (n + 1) * 8]
            r_info = struct.unpack_from(endian + "I", ent, 4)[0]
            r_type = r_info & 0xFF
            shndx = symbol_shndx(r_info >> 8)
            if r_type in (R_ARM_NONE, R_ARM_PREL31) or shndx in exidx_sections:
                dropped += 1
                continue
            kept.extend(ent)
        data[sh_offset : sh_offset + len(kept)] = kept
        struct.pack_into(endian + "I", data, off + 20, len(kept))

    open(dst, "wb").write(data)
    print(f"removed {dropped} relocations")


if __name__ == "__main__":
    main()
