/*
 * ld script for hacktribe bootloader patch
 * 0x1a1c0 bytes total OC-RAM available
 */

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

MEMORY
{
    OC_RAM (rwx)    : ORIGIN = 0x80000000, LENGTH = 0x20000
    /* OC_RAM (rwx)    : ORIGIN = 0x80005e40, LENGTH = 0x1a1c0 */
    ARM_RAM (rwx)   : ORIGIN = 0xFFFF0000, LENGTH = 0x2000
}

SECTIONS
{
    . = 0x80000000;
    /* . = 0x80005e40; */
	. = ALIGN(4);

    .rsthand :
    {
        *init.S.o    (.text)
    } > OC_RAM
 
	. = ALIGN(4);
	.text :
	{
	    *(.text)
	} > OC_RAM

	. = ALIGN(4);

	.data :
	{
	    *(.data)
	} > OC_RAM
	
	. = ALIGN(4);

	_bss_start = .;
	.bss :
	{
	    *(.bss)
	} > OC_RAM

    . = ALIGN(8);

	_bss_end = .;
	
	.stack ALIGN(8) :
	{
	    /* . += (0x80020000 - _bss_end); */
	    . += 0x1000;
	} > OC_RAM
	_stack = .;
}
