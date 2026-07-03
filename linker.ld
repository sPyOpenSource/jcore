ENTRY(loader)
OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(x86_64)

SECTIONS
{
  . = 1M;

  .text : ALIGN(8)
  {
    KEEP(*(.text.multiboot))

    *(.text*)
  }

  .rodata : ALIGN(8)
  {
    *(.rodata*)
  }

  .data  : ALIGN(8)
  {
    start_ctors = .;
    KEEP(*( .init_array ));
    KEEP(*(SORT_BY_INIT_PRIORITY( .init_array.* )));
    end_ctors = .;

    *(.data)
  }

  .bss  : ALIGN(8)
  {
    *(.bss)
  }

  /DISCARD/ : { *(.fini_array*) *(.comment) }
}
