.section ".text.boot"
#define STACK_END0 0x08000000
#define STACK_END1 0x08100000
#define STACK_END2 0x08200000
#define STACK_END3 0x08300000

.globl _start
_start:
	mrs x0, midr_el1
	mrs x0, mpidr_el1
  	and x0, x0, #0x03
	cbz x0,bss_clear
	cmp x0, #0x01
	beq c1
	cmp x0, #0x02
	beq c2
	cmp x0, #0x03
	beq c3
	b halt
bss_clear:
	ldr x0, =__bss_start
	ldr x1, =__bss_end
bss_clear_loop:
	cmp x0, x1
	beq bss_clear_exit
	str xzr, [x0], #0x08
	b bss_clear_loop
bss_clear_exit:
	ldr x0, =__stack0
	mov sp, x0 
    bl entry
c1:
	ldr x0, =__stack1
	mov sp, x0 
	mov x0, #0x01
	bl park_aux_cores
	b halt

c2:
	ldr x0, =__stack2
	mov sp, x0 
	mov x0, #0x02
	bl park_aux_cores
	b halt

c3:
	ldr x0, =__stack3
	mov sp, x0 
	mov x0, #0x03
	bl park_aux_cores
	b halt

halt: 
	b halt

