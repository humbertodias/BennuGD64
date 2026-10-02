#!/usr/bin/env python3
"""Adapt Qualcomm elf2mod.x for the big-endian Zeebo link.

CMake writes objects as *.obj, and newlib's `end` symbol must fall inside
the RW segment or elf2mod refuses the relocation.
"""

import pathlib
import sys

src = pathlib.Path(sys.argv[1]).read_text()
dst = pathlib.Path(sys.argv[2])

src = src.replace(
    '"*.o"(.text.AEEMod_Load)',
    '"*.o"(.text.AEEMod_Load)\n    "*.obj"(.text.AEEMod_Load)',
    1,
)
# elf2mod treats a symbol exactly at a PT_LOAD base as outside the segment.
# The reloc stub jumps to that base, so the gap has to be real NOPs (mov r0, r0),
# not zeros: a zero word is an illegal instruction and Zeebx stops there.
src = src.replace(
    "  . = 0x0;         /* The ro-base is at 0 */\n",
    "  . = 0x0;         /* The ro-base is at 0 */\n"
    "  .ro_pad : {\n"
    "    LONG(0xe1a00000); LONG(0xe1a00000); LONG(0xe1a00000); LONG(0xe1a00000);\n"
    "    LONG(0xe1a00000); LONG(0xe1a00000); LONG(0xe1a00000); LONG(0xe1a00000);\n"
    "  } :ER_RO\n",
    1,
)
src = src.replace(
    "  .data           : { *(.data .data.* .gnu.linkonce.d.*) } : ER_RW\n",
    "  .rw_pad : { . = . + 0x10; } :ER_RW\n"
    "  .data           : { *(.data .data.* .gnu.linkonce.d.*) }\n",
    1,
)
old = "   . = ALIGN(32 / 8);\n  }\n  \n  . = ALIGN(32 / 8);\n\n  PROVIDE (end = .);"
new = "   . = ALIGN(32 / 8);\n   PROVIDE (end = .);\n   . = . + 16;\n  }\n  \n  . = ALIGN(32 / 8);"
if old not in src:
    sys.exit("elf2mod.x layout did not match the expected .bss ending")
src = src.replace(old, new, 1)
dst.write_text(src)
