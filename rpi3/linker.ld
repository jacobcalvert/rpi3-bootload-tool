/*
 * File: linker.ld
 * Platform: Raspberry Pi 3
 * Date: 06/29/2018
 * 
 */
OUTPUT_ARCH(aarch64)
ENTRY(_start)
PAGE_SIZE = 8;
EL2_STACK_SIZE = 16K;
EL1_STACK_SIZE = 1K;
EL0_STACK_SIZE = 1K;

CANARY_SIZE = 16;

MEMORY 
{
	RAM (rwx) : ORIGIN = 0x00000000, LENGTH = 1024M /* RAM is 1g */
}

SECTIONS
{
    __start = .;
	

	
    .text :
    {
    	__text_start = .;
     	KEEP(*(.text.boot)) 
        *(.text .text.* .gnu.linkonce.t.*)
        *(.rel.text .rel.text.* .rel.gnu.linkonce.t.*)
        *(.init .init.*)
        __text_end = .;
    }> RAM
    
 

    .rodata ALIGN(PAGE_SIZE) :
    {
	    __rodata_start = .;
    	*(.rodata .rodata.*)
        *(.rel.rodata .rel.rodata.* .rel.gnu.linkonce.r.*) 
        *(.rodata)
        __rodata_end = .;
    }> RAM

 
    .data ALIGN(PAGE_SIZE):
    {
    	__data_start = .;
        *(.data .data.* .gnu.linkonce.d.*)
        *(.rel.data .rel.data.* .rel.gnu.linkonce.d.*)
        . = ALIGN(4);
       
 
    }> RAM
  


    .bss ALIGN(PAGE_SIZE) :
    { 
	    __bss_start = .;
        *(.bss .bss.* .gnu.linkonce.b.*)
        *(.rela.bss .rela.bss.* .rela.gnu.linkonce.b.*)
        *(COMMON)
        __bss_end = .;
        . = ALIGN(8);

    }> RAM
    

	
	

    __end = .;
}
__bss_size = (__bss_end - __bss_start);
