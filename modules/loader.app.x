physical_load_addr = 0x00000000;
ENTRY (_start)

PHDRS {
  text PT_LOAD FILEHDR PHDRS;
  data PT_LOAD;
}

SECTIONS
{
        . = physical_load_addr;

	. = SIZEOF_HEADERS;
        .text :
        {
				*(.init)
				*(.plt)
                *(.text)
				*(.fini)
        } :text
		
		. = physical_load_addr;
		
        .data :
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
        } :data
        .bss ALIGN (4) :
        {
                _sbss = .;
                *(.bss)
                *(COMMON)
                _ebss = .;
                . = ALIGN(4);
        } :data
        . = ALIGN(4);

        _end = .;
        PROVIDE (end = .);

		/DISCARD/ :
		{
			*(.debug*)
			*(.stab*)
			
		}
}
