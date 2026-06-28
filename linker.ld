ENTRY(_start)

SECTIONS
{
	. = 2M;

	.text :
	{
		KEEP(*(.multiboot_header))
		*(.text)
		*(.text.*)
	}

	.rodata : 
    {
        /* 3. Alle read-only data (inclusief strings uit .rodata.str1.1) sluiten hier naadloos op aan */
        *(.rodata)
        *(.rodata.*) 
    }

	.data : {
        *(.data)
    }
    edata = .; /* Markeert het einde van de data / begin van bss */

    .bss : {
        *(.bss)
        /*(COMMON)*/
    }
    end = .;   /* Markeert het einde van de BSS / begin van de heap */
}
