ENTRY(_init)

MEMORY
{
    ram : ORIGIN = 0x80000000, LENGTH = 128M
    virt : ORIGIN = 0xffffffc000000000, LENGTH = 128M
}



SECTIONS {
    .boot : { KEEP(*(.boot)) } >ram AT>ram
    .nommu : { KEEP(*(.nommu)) } >ram AT>ram

    .text : { *(.text) *(.text.*) } >virt AT>ram
    .rodata : { *(.rodata) *(.rodata.*) } >virt AT>ram
    .data : { *(.data) *(.data.*) } >virt AT>ram
	.bss  : { *(.bss) *(COMMON) } >virt AT>ram
}


/*SECTIONS {

	. = SYS_PHYS_ADDR;

	.init : { KEEP(*(.init)) } AT>ram

	. = SYS_ADDR;

	.text : { *(.text) *(.text.*) } AT>ram

	.rodata : { *(.rodata) *(.rodata.*) } AT>ram

	.data : {*(.data) *(.data.*) } AT>ram

	.bss : { *(.bss) *(COMMON) } AT>ram

}
*/
