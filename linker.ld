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

	PROVIDE(KERNEL_START = .);

	.boot : { KEEP(*(.boot)) } :re

	.text : { *(.text) *(.text.*) } :re

	.rodata : { *(.rodata) *(.rodata.*) } :r

	.data : {*(.data) *(.data.*) } : rw

	.bss : { *(.bss) *(COMMON) } :rw

	PROVIDE(KERNEL_END = .);
}

