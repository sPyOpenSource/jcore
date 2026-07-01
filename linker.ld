ENTRY(loader)
OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(x86_64)

SECTIONS
{
  . = 2M;

  .text :
  {
    *(.multiboot)
    *(.text*)
    *(.rodata)
  }

  .data  :
  {
    start_ctors = .;
    KEEP(*( .init_array ));
    KEEP(*(SORT_BY_INIT_PRIORITY( .init_array.* )));
    end_ctors = .;

    *(.data)
  }

  .bss  :
  {
    *(.bss)
  }

  /DISCARD/ : { *(.fini_array*) *(.comment) }
}
