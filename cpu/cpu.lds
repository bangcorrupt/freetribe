/*
 * Linker script for Freetribe.
 */

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

MEMORY
{
    OC_RAM (rwx)    : ORIGIN = 0x80000000, LENGTH = 0x1fffc
    KNL_PTR (r)     : ORIGIN = 0x8001fffc, LENGTH = 0x4
    ARM_RAM (rwx)   : ORIGIN = 0xFFFF0000, LENGTH = 0x2000
}

SECTIONS
{
    . = 0x80000000;
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
	
    _heap_start = .;
	.heap ALIGN(8) :
	{
	     . += 0x2000; 

	} > OC_RAM
    _heap_end = .;

	.stack ALIGN(8) :
	{
	    /* . += (0x80020000 - _heap_end); */

	     . += 0x400; 

	} > OC_RAM
	_stack = .;

    .knl_pointer ALIGN(4) :
    {
        *(.knl_pointer)
	} > KNL_PTR
}
