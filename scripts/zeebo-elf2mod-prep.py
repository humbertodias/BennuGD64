#!/usr/bin/env python3
"""Drop every ELF relocation before Qualcomm elf2mod.

The ELF is linked at the guest address Zeebx actually maps (0x10200). The
literal pools already hold those addresses. elf2mod would apply the
relocations again, sliding them down to ro-base 0, and Zeebx does not add
the load bias back. Emptying the relocation sections leaves the linked
addresses in place.

Unwind tables also add R_ARM_NONE and R_ARM_PREL31 against .ARM.exidx,
which elf2mod rejects. Dropping the whole section covers that too.
"""

import struct
import sys

SHT_REL = 9


def main():
    if len(sys.argv) not in (2, 3):
        sys.exit(f"usage: {sys.argv[0]} input.elf [output.elf]")
    src = sys.argv[1]
    dst = sys.argv[2] if len(sys.argv) == 3 else src
    data = bytearray(open(src, "rb").read())
    if data[:4] != b"\x7fELF" or data[4] != 1 or data[5] not in (1, 2):
        sys.exit("expected an ELF32")
    endian = "<" if data[5] == 1 else ">"

    e_shoff = struct.unpack_from(endian + "I", data, 32)[0]
    e_shentsize = struct.unpack_from(endian + "H", data, 46)[0]
    e_shnum = struct.unpack_from(endian + "H", data, 48)[0]

    def shdr(i):
        return struct.unpack_from(endian + "IIIIIIIIII", data, e_shoff + i * e_shentsize)

    dropped = 0
    for i in range(e_shnum):
        if shdr(i)[1] != SHT_REL:
            continue
        off = e_shoff + i * e_shentsize
        sh_size = shdr(i)[5]
        dropped += sh_size // 8
        struct.pack_into(endian + "I", data, off + 20, 0)

    open(dst, "wb").write(data)
    print(f"removed {dropped} relocations")


if __name__ == "__main__":
    main()
