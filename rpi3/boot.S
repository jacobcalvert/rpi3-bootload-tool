.section ".text.boot"
#define STACK_END 0x08000000

.globl _start
_start:
bss_clear:
	ldr x0, =__bss_start
	ldr x1, =__bss_end
bss_clear_loop:
	cmp x0, x1
	beq bss_clear_exit
	str xzr, [x0], #0x08
	b bss_clear_loop
bss_clear_exit:
    mrs x0,mpidr_el1
  	and x0, x0, #0x03
    cbz x0,core0
	b halt
core0:
    mov sp,#STACK_END
    bl entry

halt: 
	b halt

