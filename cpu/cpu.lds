/*
 * Linker script for Freetribe.
 */

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

HEAP_SIZE = 0x1000;
STACK_SIZE = 0x1000;
VECTOR_TABLE_SIZE = 0x80;

MEMORY
{
    OC_RAM (rwx)    : ORIGIN = 0x80000000, LENGTH = 0x1fffc
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
	
	.heap ALIGN(8) :
	{
        _heap_start = .;

	     . += HEAP_SIZE;

        _heap_end = .;
	} > OC_RAM

	.stack :
	{
        . = ALIGN(8); 
        _stack_low = .;


	     . += STACK_SIZE; 

        _stack = .;
	} > OC_RAM

    /*
     * Vector table is moved to 
     * ARM RAM by startup code.
     */
	.vector_table :
	{
        . = ALIGN(8); 
        _vector_table = .;


	    . += VECTOR_TABLE_SIZE; 

	} > ARM_RAM


}
