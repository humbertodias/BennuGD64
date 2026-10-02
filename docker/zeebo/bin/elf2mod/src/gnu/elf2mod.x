/*
** GCC ELF Linker script for elf2mod compatible BREW modules
*/

OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
SEARCH_DIR("=/lib");


PHDRS
{
    ER_RO PT_LOAD FLAGS(5) /* .ctors and .dtors erroneously contribute PF_W */;
    ER_RW PT_LOAD ;
}

SECTIONS
{
/*-------------------------------------------------------------------------*/
  /* Read-only sections (code & data) */

  . = 0x0;         /* The ro-base is at 0 */
  
  __module_start__ = .;     /* Use this to compute offsets */

  __text_start_offset__ = . - __module_start__;

    .text           :
  {
    "*.o"(.text.AEEMod_Load)
    *(.text .stub .text.* .gnu.linkonce.t* .glue_7t .glue_7)
    *(.ARM.extab* .gnu.linkonce.armextab.*)
	. = ALIGN(4);
    KEEP(*(.init))

    . = ALIGN(4);
    __preinit_array_start = .;
    KEEP (*(.preinit_array))
	__preinit_array_end = .;

    . = ALIGN(4);
    __init_array_start = .;
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array))
    __init_array_end = .;

    . = ALIGN(4);
    KEEP(*(.fini))

    . = ALIGN(4);
    __fini_array_start = .;
    KEEP (*(.fini_array))
    KEEP (*(SORT(.fini_array.*)))
    __fini_array_end = .;

    . = ALIGN(0x4);
    KEEP (*crtbegin.o(.ctors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*crtend.o(.ctors))

    . = ALIGN(0x4);
    KEEP (*crtbegin.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*crtend.o(.dtors))

  } :ER_RO =0

  __text_end_offset__ = . - __module_start__;

  . = ALIGN(32 / 8);

  PROVIDE_HIDDEN (__exidx_start = .);
  .ARM.exidx :
  {
    *(.ARM.exidx* .gnu.linkonce.armexidx.*)
  } 
  PROVIDE_HIDDEN (__exidx_end = .);
  . = ALIGN(32 / 8);

  .rodata         : { *(.rodata .rodata.* .gnu.linkonce.r.*) }

  . = ALIGN(32 / 8);

  .rodata1        : { *(.rodata1) }

  . = ALIGN(32 / 8);
  
  /* End of read only data section */
/*-------------------------------------------------------------------------*/
  
  /* Beginning of read/write section (data) */
  
  .data           : { *(.data .data.* .gnu.linkonce.d.*) } : ER_RW

  . = ALIGN(32 / 8);

  .data1          : { *(.data1) }

  . = ALIGN(32 / 8);

  /* End of read-write section */
  
  /* Beginning of bss section */

  .bss            :
  {
   *(.bss .bss.* .gnu.linkonce.b.*)
   *(COMMON)
   /* Align here to ensure that the .bss section occupies space up to
      _end.  Align after .bss to ensure correct alignment even if the
      .bss section disappears because there are no input sections.  */
   . = ALIGN(32 / 8);
  }
  
  . = ALIGN(32 / 8);

  PROVIDE (end = .);    /* For newlib libnosys.a version of sbrk */
  
  /* End of read/write section */
/*-------------------------------------------------------------------------*/

  /* Begin debug info */

  /* DWARF debug sections.
     Symbols in the DWARF debugging sections are relative to the beginning
     of the section so we begin them at 0.  */
  /* DWARF 1 */
  .debug          0 : { *(.debug) } : NONE
  .line           0 : { *(.line) }
  /* GNU DWARF 1 extensions */
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames) }
  /* DWARF 1.1 and DWARF 2 */
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  /* DWARF 2 */
  .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo) }
  /* SGI/MIPS DWARF 2 extensions */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }
  
  /* Discarded sections */
  
  /DISCARD/ : {*(.stack)} : NONE            /* Leave out the stack */
  
  /DISCARD/ : {*(.stab* .comment*)} : NONE  /* Leave out stabs debug */
}
