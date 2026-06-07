ENTRY(_start)

SECTIONS
{
	. = 1M;

	.boot :
	{
		KEEP(*(.multiboot_header))
	}

	.text :
	{
		*(.text)
	}

	.data : {
        *(.data)
    }
    edata = .; /* Markeert het einde van de data / begin van bss */

    .bss : {
        *(.bss)
        *(COMMON)
    }
    end = .;   /* Markeert het einde van de BSS / begin van de heap */
}
