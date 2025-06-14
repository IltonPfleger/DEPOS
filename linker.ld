MEMORY {
}


PHDRS {
	/*READ | WRITE | EXECUTE*/
	r PT_LOAD FLAGS(4);
	re PT_LOAD FLAGS(5);
	rw PT_LOAD FLAGS(6);
}



SECTIONS {
	. = 0x80000000;

	PROVIDE(__KERNEL_START__ = .);

	.boot : { KEEP(*(.boot)) } :re

	.text : { *(.text) *(.text.*) } :re

	.rodata : { *(.rodata) *(.rodata.*) } :r

	.data : {*(.data) *(.data.*) } : rw

	.bss : { *(.bss) *(COMMON) } :rw

	/*. = 0x80200000;*/
	PROVIDE(__KERNEL_END__ = .);
}

/*.boot : { KEEP(*(.boot))  } :re
.text : { *(.text*) } :re
.data : { *(.data)  } :rw
.bss  : { *(.bss)   } :rw
PROVIDE(__KERNEL_END__ = .);*/

