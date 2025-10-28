/*
 * Linker script for Freetribe.
 */

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

HEAP_SIZE = 0x800;
STACK_SIZE = 0x1f80;
VECTOR_TABLE_SIZE = 0x80;

MEMORY
{
    OC_RAM (rwx)    : ORIGIN = 0x80000000, LENGTH = 0x20000
    ARM_RAM (rwx)   : ORIGIN = 0xFFFF0000, LENGTH = 0x2000
    DDR2_RAM (rw)   : ORIGIN = 0xc0000000, LENGTH = 64M /* sk hynix h5ps5162kfr-s5c chip maximum is 64MB */
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

    /*
     * Put the stack in ARM RAM for now.
     * This is VERY BAD as it grows 
     * toward the vector table.
     *
     * Once the app is compiled standalone
     * and loaded into external DDR,
     * the stack goes back in On-Chip RAM
     * and we use ARM RAM for interrupt handling.
     */
	.stack :
	{
        . = ALIGN(8); 
        _stack_low = .;


	     . += STACK_SIZE; 

        _stack = .;
	} > ARM_RAM


	_ddr2_start = ORIGIN(DDR2_RAM);
	_ddr2_end = ORIGIN(DDR2_RAM) + LENGTH(DDR2_RAM);

}
