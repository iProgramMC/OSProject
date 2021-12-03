/* Linker script for the OS */

/* Entry point for application. This will be the assembler function, since we have some setup to do. */
/* ENTRY (_start) */
ENTRY (Entry)

/* Here is where all of the sections of the kernel are defined. */
SECTIONS
{
	/* Begin loading at 0x100000, as that's where GRUB will place our data. */
	. = 1M;
	/* Put the multiboot header, since we need that to be as early in the kernel
	   as possible, or the bootloader cannot recognize this ELF. 
	   After that we can put the code (.text) */
	.text BLOCK(4K) : ALIGN(4K) 
	{
		*(.multiboot)
		*(.text)
	}
	
	/* Read only data */
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	}
	
	/* Variables with initial values or non-constant arrays */
	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}
	
	/* Read-write data (uninitialized) and stack) */
	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
		/* simple kmalloc before we get paging-style malloc working */
		*(.heapstart) 
	}
 
	/* The compiler may produce other sections, by default it will put them in
	   a segment with the same name. Simply add stuff here as needed. */
}