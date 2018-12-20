/*
 * File boot.S
 * Author: Jacob Calvert <jcalvert@jacobncalvert.com>
 * 
 * Revision History
 * ------------------
 * 0.01 - initial revision
 */
.section ".text.boot"

.globl _start
/*
 * all four cores will start
 * we read the core id and make some decisions based on that
 * if we are core 0, we clear the BSS and go to the main entry
 * if we're the other cores, we set up a stack and go to the aux 
 * core parking routine
 */
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

