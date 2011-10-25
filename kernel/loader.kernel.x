physical_load_addr = 0x00100000;
ENTRY (_start)

SECTIONS
{
        . = physical_load_addr;

        .text :
        {
				*(.init)
				*(.plt)
                *(.text)
				*(.fini)
        }
        .data ALIGN (0x1000) :
        {
				*(.interp)
				*(.note*)
				*(.hash)
				*(.dynsym)
				*(.dynstr)
				*(.gnu*)
				*(.rel.*)
				*(.rodata*)
				*(.eh_frame*)
				*(.data)
				*(.dynamic)
				*(.ctors)
				*(.dtors)
				*(.jcr)
				*(.got*)
				*(.gcc*)
        }
        .bss ALIGN (4) :
        {
                _sbss = .;
                *(.bss)
                *(COMMON)
                _ebss = .;
                . = ALIGN(4);
        }
        . = ALIGN(4);

        _end = .;
        PROVIDE (end = .);

		/DISCARD/ :
		{
			*(.debug*)
			*(.stab*)
			
		}
}
