ENTRY(start)

SECTIONS
{
	. = 1M;

	.boot :
	{
		KEEP(*(.boot_hdr))
	}

	.text :
	{
		*(.text)
	}
}
